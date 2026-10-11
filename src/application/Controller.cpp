//
// Created by Chris Davey on 11/10/2026.
//

#include "Controller.h"

#include <iostream>

/**
     * construct with the base client
     * @param client
     */
Controller::Controller(std::shared_ptr<sdrcomms::Client> client,
                       std::shared_ptr<sdrcomms::MessageQueue<sdrcomms::ControlMessage> > messageQueue) {
    this->client = client;
    this->messageQueue = messageQueue;
}

/**
     * start scanning
     * @return
     */
bool Controller::start() {
    if (running.load(std::memory_order_relaxed)) {
        return true;
    }
    running.store(true, std::memory_order_relaxed);
    worker_thread = std::thread([this]() {
        this->worker();
    });
    control_thread = std::thread([this]() {
        this->controller();
    });
    return true;
}

/**
 * stop scanning.
 * @return
 */
bool Controller::stop() {
    if (running.load(std::memory_order_relaxed)) {
        running.store(false, std::memory_order_relaxed);
        if (worker_thread.joinable()) {
            worker_thread.join();
        }
        if (control_thread.joinable()) {
            control_thread.join();
        }
    }
}

/**
     * run the control algorithm.
     */
void Controller::controller() {
    while (running.load(std::memory_order_relaxed)) {
        if (!autoScan.load(std::memory_order_relaxed)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            continue;
        }
        auto now = std::chrono::steady_clock::now();
        auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastScanChange).count();
        if (delta > sdrcomms::FREQUENCY_SCAN_CHANGE_MILLIS) {
            if (current_freq + scan_direction*sdrcomms::FREQUENCY_STEP >= max_freq) {
                scan_direction = -1;
            } else if (current_freq + scan_direction*sdrcomms::FREQUENCY_STEP <= min_freq) {
                scan_direction = 1;
            }
            current_freq += scan_direction*sdrcomms::FREQUENCY_STEP;
            client->SetCenterFrequency(current_freq);
            std::cout << "Setting frequency to " << current_freq << std::endl;
            lastScanChange = now;
        }
    }
}

/**
     * runtime processing
     */
void Controller::worker() {
    while (running.load(std::memory_order_relaxed)) {
        sdrcomms::ControlMessage controlMessage{};
        if (messageQueue->pop(controlMessage)) {
            // we can perform work on this message.
            switch (controlMessage.type) {
                case sdrcomms::MessageType::MSG_AUTO_SCAN:
                    autoScan.store(true, std::memory_order_relaxed);
                    break;
                case sdrcomms::MessageType::MSG_STOP_SCAN:
                    autoScan.store(false, std::memory_order_relaxed);
                    break;
                case sdrcomms::MessageType::MSG_SET_FREQ:
                    autoScan.store(false, std::memory_order_relaxed);
                    if (controlMessage.freq != 0) {
                        client->SetCenterFrequency(controlMessage.freq);
                    }
                    break;
                default:
                    //
                    break;
            }
        }
        std::this_thread::sleep_for(std::chrono::microseconds(200));
    }
}
