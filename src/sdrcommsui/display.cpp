//
// Created by Chris Davey on 4/10/2026.
//


#include "include/display.h"

#include <iostream>

#include "include/SpectrumWaterfall.h"

namespace sdrcommsui {
    /**
         * initialise the display
         * @param udpServer
         * @param port
         * @param sample_size
         */
    Display::Display(std::string udpServer, int port, size_t sample_size, int width, int height) {
        this->udpServer = udpServer;
        this->port = port;
        this->sampleSize = sample_size;
        this->netClient = std::make_unique<NetClient>(udpServer, port, sample_size);

        this->lineSketchQueue = std::make_shared<sdrcomms::ReceiveQueue>(MAX_QUEUE_SIZE, sample_size);
        this->waterfallSketchQueue = std::make_shared<sdrcomms::ReceiveQueue>(MAX_QUEUE_SIZE, sample_size);

        this->sketch = std::make_shared<SpectrumWaterfall>(width,
                                    height,
                                    sample_size,
                                    this->waterfallSketchQueue);
        this->lineSketch = std::make_shared<FFTLinePlot>(
            width,
            height,
            sample_size,
            this->lineSketchQueue);
        this->app = std::make_unique<drawing::DrawingApp>(SDL_INIT_VIDEO,
                                                          "Spectrum Waterfall",
                                                          false,
                                                          width,
                                                          height,
                                                          this->sketch,
                                                          this->lineSketch);
    }

    void Display::run() {
        if (!this->connect()) {
            std::cerr << "Could not connect to: " << udpServer << " " << port << std::endl;
        }
        running.store(true, std::memory_order_relaxed);
        this->subscibe();
        // sleep for a while to allow the client to start and then initialise the UI
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        this->receive_thread = std::thread(
           [this]() {
               this->receive_blocks();
           }
       );
        // might need to run this in the background.
        this->lineSketch->init();
        this->sketch->init();
        this->app->Run();

    }

    void Display::stop() {
        if (running.load(std::memory_order_relaxed)) {
            this->unsubscribe();
            this->disconnect();
            running.store(false, std::memory_order_relaxed);
        }
        // stop queue processing in the sketches.
        this->lineSketch->stop();
        this->sketch->stop();

        if (this->receive_thread.joinable()) {
            this->receive_thread.join();
        }

    }

    /**
        * receiver thread updates blocks
        */
    void Display::receive_blocks() {
        while (running.load(std::memory_order_relaxed)) {
            sdrcomms::IQBlock block;
            if (this->netClient->getReceiveQueue()->pop(block)) {
                // copy the data into the child queues.
                std::vector<std::complex<float>> copySamples = block.samples;
                this->waterfallSketchQueue->push(block.timestampMs, block.channel, copySamples, copySamples.size(), block.centerFreq);
                copySamples = block.samples;
                this->lineSketchQueue->push(block.timestampMs, block.channel, copySamples, copySamples.size(), block.centerFreq);

            }
        }
    }

    /**
     * connect to the server
     */
    bool Display::connect() {
        return this->netClient->connect_server();
    }

    /**
     * subscribe to messages
     * @return
     */
    bool Display::subscibe() {
        return this->netClient->subscribe();
    }

    /**
     * unsubscribe
     */
    bool Display::unsubscribe() {
        return this->netClient->unsubscribe();
    }

    /**
     * disconnect from the server
     * @return
     */
    bool Display::disconnect() {
        return this->netClient->disconnect();
    }
} // sdrcommsui
