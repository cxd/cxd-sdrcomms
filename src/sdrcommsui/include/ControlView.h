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
#include "sketch.h"

namespace sdrcommsui {
    class ControlView : public drawing::Sketch  {
    public:
        ControlView(std::string udpServer, int port, double centerFreq,
            std::shared_ptr<sdrcomms::MessageQueue<sdrcomms::ControlMessage>> inboundMessageQueue,
            std::shared_ptr<sdrcomms::MessageQueue<sdrcomms::ControlMessage>> outboundMessageQueue);


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
        bool autoScan;
        long long timestampDb;
        double centerFreq;
        std::string classLabel;
        double confidence;


        std::atomic<bool> isInit;
        std::atomic<bool> isRunning;
        std::shared_mutex updateMutex;
        std::thread receive_thread;

        std::thread inbound_msg_thread;

        std::shared_ptr<sdrcomms::MessageQueue<sdrcomms::ControlMessage>> inboundMessageQueue;
        std::shared_ptr<sdrcomms::MessageQueue<sdrcomms::ControlMessage>> outboundMessageQueue;
    };
}

#endif //SDRCOMMS_CONTROLVIEW_H
