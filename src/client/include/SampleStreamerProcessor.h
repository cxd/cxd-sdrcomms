//
// Created by Chris Davey on 27/9/2026.
//

#ifndef SDRCOMMS_SAMPLESTREAMERPROCESSOR_H
#define SDRCOMMS_SAMPLESTREAMERPROCESSOR_H
#include <atomic>
#include <memory>
#include <thread>
#include <netinet/in.h>

#include "constants.h"
#include "SampleProcessor.h"
#include "ReceiveQueue.h"
namespace sdrcomms {

    struct NetClient {
        sockaddr_in address;
        std::chrono::time_point<std::chrono::steady_clock> heartbeat;
        // compare two clients.
        bool operator==(const NetClient& other) const {
            return address.sin_addr.s_addr == other.address.sin_addr.s_addr &&
                address.sin_port == other.address.sin_port;
        }
    };


    static_assert(std::is_trivially_copyable<NetClient>::value, "Client must be trivially copyable!");

    /**
     * a streaming processor that will forward samples to a connected client.
     */
    class SampleStreamerProcessor : public SampleProcessor {
    public:
        SampleStreamerProcessor(std::string udpServerAddress, int udpPort);

        void process(const IQBlock &block) override;

        /**
         * start the server process.
         * this is called from outside of the sdr client before passing into the client.
         */
        void startServer();

        /**
         * stop the server process.
         */
        void stopServer();

    private:
        std::string udpServerAddress;
        int udpPort;
        std::atomic<bool> hasClient {false};
        std::atomic<bool> running {false};

        // internal server socket file descriptor.
        int server_socket;
        // internal receive queue
        std::shared_ptr<ReceiveQueue> receiveQueue;

        // registered clients
        std::shared_ptr<std::vector<NetClient> > active_clients { std::make_shared<std::vector<NetClient>> () };


        std::thread client_thread;
        std::thread stream_thread;
        /**
         * wait for clients to connect
         */
        void handle_client_connections();

        /**
         * iterate over each client and stream samples
         */
        void stream_samples();

        /**
         * cleanup expired clients.
         */
        void cleanup_expired_clients();

        /**
         * remove client.
         */
        void remove_client(const NetClient& client);

    };
} // sdrcomms

#endif //SDRCOMMS_SAMPLESTREAMERPROCESSOR_H
