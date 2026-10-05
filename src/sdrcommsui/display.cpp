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

        this->sketch = std::make_shared<SpectrumWaterfall>(width,
                                    height,
                                    sample_size,
                                    this->netClient->getReceiveQueue());
        this->sketch->init();
        this->app = std::make_unique<drawing::DrawingApp>(SDL_INIT_VIDEO,
                                                          "Spectrum Waterfall",
                                                          false,
                                                          width,
                                                          height,
                                                          this->sketch);
    }

    void Display::run() {
        if (!this->connect()) {
            std::cerr << "Could not connect to: " << udpServer << " " << port << std::endl;
        }
        running.store(true, std::memory_order_relaxed);
        this->subscibe();
        // might need to run this in the background.
        this->sketch->init();
        this->app->Run();
    }

    void Display::stop() {
        if (running.load(std::memory_order_relaxed)) {
            this->unsubscribe();
            this->disconnect();
            running.store(false, std::memory_order_relaxed);
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
