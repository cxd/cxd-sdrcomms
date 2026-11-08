//
// Created by Chris Davey on 7/11/2026.
//

#ifndef SDRCOMMS_CONTROLVIEW_H
#define SDRCOMMS_CONTROLVIEW_H
#include <string>
#include <atomic>
#include <shared_mutex>
#include <thread>

#include "ControlMessage.h"
#include "MessageQueue.h"
#include "ReceiveQueue.h"
#include "sketch.h"

namespace sdrcommsui {
    class ControlView : public drawing::Sketch  {
    public:
        ControlView(std::string udpServer, int port, double centerFreq,
            std::shared_ptr<sdrcomms::ReceiveQueue> receiveQueue,
            std::shared_ptr<sdrcomms::MessageQueue<sdrcomms::ControlMessage>> inboundMessageQueue,
            std::shared_ptr<sdrcomms::MessageQueue<sdrcomms::ControlMessage>> outboundMessageQueue);

        /**
         * initialise the threads
         * @return
         */
        bool init();

        /**
         * stop reading queues
         */
        void stop();

        /**
         * Draw the spectrum waterfall.
         * @param renderer
         * @param width
         * @param height
         * @param timestep
         * @param frame
         * @param fps
         */
        void DrawSketch(SDL_Renderer *renderer, int width, int height, int timestep, int frame, int fps) override;


    private:
        std::string udpServer;
        int port;
        bool autoScan = true;
        long long timestampMs;
        double centerFreq;
        long long classTimestampMs = 0;
        std::string classLabel = "";
        double confidence = 0;


        std::atomic<bool> isInit;
        std::atomic<bool> isRunning;
        std::shared_mutex updateMutex;
        std::thread receive_thread;

        std::thread inbound_msg_thread;

        std::shared_ptr<sdrcomms::ReceiveQueue> receiveQueue;

        std::shared_ptr<sdrcomms::MessageQueue<sdrcomms::ControlMessage>> inboundMessageQueue;
        std::shared_ptr<sdrcomms::MessageQueue<sdrcomms::ControlMessage>> outboundMessageQueue;

        /**
         * format a timestamp as HH:MM:SS.sss
         * @param ts
         * @return
         */
        std::string format_ts(long long ts);

        /**
         * use current settings to send the control frequency message.
         */
        void send_control_frequency_msg();

        /**
         * consume inbound messages
         */
        void receive_inbound_messages();

        /**
         * receive udp stream
         */
        void receive_stream();
    };
}

#endif //SDRCOMMS_CONTROLVIEW_H
