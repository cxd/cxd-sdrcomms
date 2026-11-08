//
// Created by Chris Davey on 7/11/2026.
//

#include "include/ControlView.h"

#include <imgui.h>
#include <iostream>

namespace sdrcommsui {
    ControlView::ControlView(std::string udpServer, int port, double centerFreq,
                             std::shared_ptr<sdrcomms::ReceiveQueue> receiveQueue,
                             std::shared_ptr<sdrcomms::MessageQueue<sdrcomms::ControlMessage> > inboundMessageQueue,
                             std::shared_ptr<sdrcomms::MessageQueue<sdrcomms::ControlMessage> > outboundMessageQueue) {
        this->udpServer = udpServer;
        this->port = port;
        this->centerFreq = centerFreq;
        this->receiveQueue = receiveQueue;
        this->inboundMessageQueue = inboundMessageQueue;
        this->outboundMessageQueue = outboundMessageQueue;
    }


    /**
         * initialise the threads
         * @return
         */
    bool ControlView::init() {
        isRunning.store(true, std::memory_order_relaxed);

        receive_thread = std::thread(
            [this]() {
                this->receive_stream();
            });

        inbound_msg_thread = std::thread(
            [this] {
                this->receive_inbound_messages();
            });
    }

    /**
     * stop reading queues
     */
    void ControlView::stop() {
        isRunning.store(false, std::memory_order_relaxed);
        if (receive_thread.joinable()) {
            receive_thread.join();
        }
        if (inbound_msg_thread.joinable()) {
            inbound_msg_thread.join();
        }
    }

    /**
        * Draw the spectrum waterfall.
        * @param renderer
        * @param width
        * @param height
        * @param timestep
        * @param frame
        * @param fps
        */
    void ControlView::DrawSketch(SDL_Renderer *renderer, int width, int height, int timestep, int frame, int fps) {
        // draw ImGui Controls.
        std::shared_lock<std::shared_mutex> lock(this->updateMutex);
        ImGui::Text("Server: %s", udpServer.c_str());
        ImGui::Text("Port: %d", port);
        ImGui::Text("Render FPS: %d", fps);
        ImGui::Text("Current Timestamp: %s", format_ts(timestampMs).c_str());
        std::string centerFreqStr = std::to_string(centerFreq);
        //ImGui::InputText("Center Frequency Hz", centerFreqStr.c_str());
        ImGui::Text("Center Frequency: %f", centerFreq);
        bool scanFrequency = autoScan;
        bool scanModified = ImGui::Checkbox("Scan Frequency Automatically", &scanFrequency);

        if (classLabel.length() > 0 && confidence > 0) {
            ImGui::Text("Class Timestamp: %s", format_ts(classTimestampMs).c_str());
            ImGui::Text("Modulation Class: %s", classLabel.c_str());
            ImGui::Text("Classification Confidence: %f", confidence);
        } else {
            ImGui::Text("Class Timestamp: %lld", 0);
            std::string emptyStr = "";
            ImGui::Text("Modulation Class: %s", emptyStr.c_str());
            ImGui::Text("Classification Confidence: %f", 0);
        }
        // check if the autoScan setting has changed from true to false.
        if (scanModified && !scanFrequency && autoScan) {
            autoScan = false;
            send_control_frequency_msg();
        } else if (scanModified && scanFrequency && !autoScan) {
            autoScan = true;
            send_control_frequency_msg();
        }
    }

    /**
         * format a timestamp as HH:MM:SS.sss
         * @param ts
         * @return
         */
    std::string ControlView::format_ts(long long ts) {
        if (ts == 0) {
            return "0";
        }
        std::chrono::milliseconds ms_duration(ts);
        auto seconds = std::chrono::duration_cast<std::chrono::seconds>(ms_duration);
        std::time_t time_sec = seconds.count();
        long long remaining_ms = (ms_duration - seconds).count();
        std::tm* time_info = std::localtime(&time_sec);
        std::ostringstream oss;
        oss << std::put_time(time_info, "%H:%M:%S") << "." << std::setfill('0') << std::setw(3) << remaining_ms;
        return oss.str();
    }


    /**
         * use current settings to send the control frequency message.
         */
    void ControlView::send_control_frequency_msg() {
        if (autoScan) {
            sdrcomms::ControlMessage msg{
                sdrcomms::MessageType::MSG_AUTO_SCAN,
                sdrcomms::MessageResult::MSG_NONE,
                centerFreq,
            };
            this->outboundMessageQueue->push(msg);
        } else {
            sdrcomms::ControlMessage msg{
                sdrcomms::MessageType::MSG_STOP_SCAN,
                sdrcomms::MessageResult::MSG_NONE,
                centerFreq
            };
            this->outboundMessageQueue->push(msg);
        }
    }


    void ControlView::receive_stream() {
        while (isRunning.load(std::memory_order_relaxed)) {
            sdrcomms::IQBlock block;
            if (receiveQueue->pop(block)) {
                std::shared_lock<std::shared_mutex> lock(this->updateMutex);
                centerFreq = block.centerFreq;
                timestampMs = block.timestampMs;
            }
        }
    }

    void ControlView::receive_inbound_messages() {
        while (isRunning.load(std::memory_order_relaxed)) {
            sdrcomms::ControlMessage msg;
            if (inboundMessageQueue->pop(msg)) {
                // we can check what kind of message this is.
                std::shared_lock<std::shared_mutex> lock(this->updateMutex);
                if (msg.type == sdrcomms::MessageType::MSG_CLASSIFY) {
                    classLabel = msg.classLabel;
                    confidence = msg.confidence;
                    classTimestampMs = msg.timestampMs;
                    std::cout << "ControlView::classification: " << msg.message_type_to_string() <<
                            " Class: " << classLabel <<
                            " Confidence: " << confidence << std::endl;
                }
            }
        }
    }
}
