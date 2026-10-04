//
// Created by Chris Davey on 4/10/2026.
//

#ifndef SDRCOMMS_DISPLAY_H
#define SDRCOMMS_DISPLAY_H
#include <memory>
#include "netclient.h"
namespace sdrcommsui {
    class Display {
    public:
        /**
         * initialise the display
         * @param udpServer
         * @param port
         * @param sample_size
         */
        Display(std::string udpServer, int port, size_t sample_size);

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

    };
} // sdrcommsui

#endif //SDRCOMMS_DISPLAY_H
