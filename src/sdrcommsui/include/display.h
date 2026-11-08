//
// Created by Chris Davey on 4/10/2026.
//

#ifndef SDRCOMMS_DISPLAY_H
#define SDRCOMMS_DISPLAY_H
#include <memory>

#include "ControlMessage.h"
#include "ControlView.h"
#include "DrawingApp.h"
#include "FFTLinePlot.h"
#include "MessageQueue.h"
#include "netclient.h"
#include "sketch.h"
#include "SpectrumWaterfall.h"

namespace sdrcommsui {
    class Display {
    public:
        /**
         * initialise the display
         * @param udpServer
         * @param port
         * @param sample_size
         */
        Display(std::string udpServer,
            int port,
            size_t sample_size,
            int width,
            int height,
            std::shared_ptr<sdrcomms::MessageQueue<sdrcomms::ControlMessage>> inboundQueue,
            std::shared_ptr<sdrcomms::MessageQueue<sdrcomms::ControlMessage>> outputBoundQueue);

        /**
         * run the display
         */
        void run();

        /**
         * stop the display
         */
        void stop();

        /**
         * connect to the server
         */
        bool connect();

        /**
         * subscribe to messages
         * @return
         */
        bool subscibe();

        /**
         * unsubscribe
         */
        bool unsubscribe();

        /**
         * disconnect from the server
         * @return
         */
        bool disconnect();
    private:
        std::string udpServer;
        int port;
        size_t sampleSize;
        std::unique_ptr<NetClient> netClient;
        std::atomic<bool> running;
        // the receive queue is used to relay copies of the IQ Block data into the respective sketches
        std::shared_ptr<sdrcomms::ReceiveQueue> receiveQueue;

        std::shared_ptr<sdrcomms::ReceiveQueue> lineSketchQueue;
        std::shared_ptr<sdrcomms::ReceiveQueue> waterfallSketchQueue;
        std::shared_ptr<sdrcomms::ReceiveQueue> controlSketchQueue;

        std::shared_ptr<ControlView> controlView;

        std::thread receive_thread;

        std::shared_ptr<drawing::DrawingApp> app;
        std::shared_ptr<SpectrumWaterfall> sketch;
        std::shared_ptr<FFTLinePlot> lineSketch;

        const size_t MAX_QUEUE_SIZE = 256;

        /**
        * receiver thread updates blocks
        */
        void receive_blocks();


    };
} // sdrcommsui

#endif //SDRCOMMS_DISPLAY_H
