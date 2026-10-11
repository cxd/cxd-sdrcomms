//
// Created by Chris Davey on 11/10/2026.
//

#ifndef SDRCOMMS_CONTROLLER_H
#define SDRCOMMS_CONTROLLER_H
#include "client.h"
#include "ControlMessage.h"
#include "MessageQueue.h"

class Controller {
public:
    /**
     * construct with the base client
     * @param client
     * @param messageQueue
     * TODO: later we will add outgoing message queue when a result has been captured.
     * TODO: we will also need a notification queue to receive detected classifications emitted from a signal classifier.
     */
    Controller(std::shared_ptr<sdrcomms::Client> client,
               std::shared_ptr<sdrcomms::MessageQueue<sdrcomms::ControlMessage> > messageQueue);

    /**
     * start scanning
     * @return
     */
    bool start();

    /**
     * stop scanning.
     * @return
     */
    bool stop();

private:
    std::shared_ptr<sdrcomms::Client> client;

    std::shared_ptr<sdrcomms::MessageQueue<sdrcomms::ControlMessage> > messageQueue;

    std::thread worker_thread;


    std::thread control_thread;

    std::atomic<bool> running = false;
    // 70 MHz
    double min_freq = 7e6;
    // 6 GhZ
    double max_freq = 6e7;
    std::chrono::steady_clock::time_point lastScanChange = std::chrono::steady_clock::now();
    double current_freq = sdrcomms::DEFAULT_CENTER_FREQUENCY;
    int scan_direction = 1;

    std::atomic<bool> autoScan = true;

    /**
     * runtime processing
     */
    void worker();

    /**
     * run the control algorithm.
     */
    void controller();
};


#endif //SDRCOMMS_CONTROLLER_H
