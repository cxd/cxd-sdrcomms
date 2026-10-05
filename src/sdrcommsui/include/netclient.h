//
// Created by Chris Davey on 29/9/2026.
//

#ifndef SDRCOMMS_NETCLIENT_H
#define SDRCOMMS_NETCLIENT_H
#include <string>
#include "ReceiveQueue.h"
#include <atomic>
#include <chrono>
#include <thread>
#include <netinet/in.h>

namespace sdrcommsui {

    struct VectorReadBuffer : std::streambuf {
        VectorReadBuffer(std::vector<uint8_t>& buffer) {
            char* buffer_ptr = const_cast<char*>(reinterpret_cast<const char*>(buffer.data()));
            setg(buffer_ptr, buffer_ptr, buffer_ptr + buffer.size());
        }
    };

    /**
     * a frame chunk to support reordering received data.
     */
    struct FrameChunk {
        int chunk_index;
        int total_chunks;
        std::vector<uint8_t> data;
    };

    class NetClient {
    public:
        /**
         * initialise with connection to udp server and port.
         * @param udpServer
         * @param port
         */
        NetClient(std::string udpServer, int port, size_t sample_size);

        /**
         * accessor for the receive queue
         * @return
         */
        std::shared_ptr<sdrcomms::ReceiveQueue> getReceiveQueue();

        /**
         * open a connection to the udp server
         * @return
         */
        bool connect_server();

        /**
         * subscribe to the udp server to receive samples
         * this will cause samples to start streaming into the
         * receiveQueue.
         * @return
         */
        bool subscribe();

        /**
         * unsubscribe from the udp server.
         * this will stop samples streaming into the receive queue.
         * @return
         */
        bool unsubscribe();

        /**
         * disconnect from the server.
         * @return
         */
        bool disconnect();


    private:
        std::string udpServer;
        int port;
        int server_socket;

        size_t sample_size;
        sockaddr_in server_address;
        std::shared_ptr<sdrcomms::ReceiveQueue> receiveQueue;
        std::atomic<bool> isConnected { false };
        std::atomic<bool> isSubscribed { false };
        std::chrono::steady_clock::time_point lastHeartbeat;
        std::thread receive_thread;
        std::thread heartbeat_thread;
        /**
         * send a control message
         */
        void send_msg(std::string msg);


        /**
         * receive samples from the udp server
         * @param server_socket
         */
        void receive_stream();

        /**
         * send heartbeats to the udp server.
         * this repeats the register request.
         * @param server_socket
         */
        void heartbeat();

    };
} // sdrcommsui

#endif //SDRCOMMS_NETCLIENT_H
