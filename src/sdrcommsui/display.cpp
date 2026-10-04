//
// Created by Chris Davey on 4/10/2026.
//


#include "include/display.h"

#include <iostream>

namespace sdrcommsui {
    /**
         * initialise the display
         * @param udpServer
         * @param port
         * @param sample_size
         */
    Display::Display(std::string udpServer, int port, size_t sample_size) {
        this->udpServer = udpServer;
        this->port = port;
        this->sampleSize = sample_size;
        this->netClient = std::make_unique<NetClient>(udpServer, port, sample_size);
    }

    void Display::run() {
        if (!this->connect()) {
            std::cerr << "Could not connect to: " << udpServer << " " << port << std::endl;
        }
        running.store(true, std::memory_order_relaxed);
        this->subscibe();
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
        return this->netClient->connect();
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
