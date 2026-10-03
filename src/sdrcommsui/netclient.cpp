//
// Created by Chris Davey on 29/9/2026.
//

#include "include/netclient.h"

#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#include "constants.h"

namespace sdrcommsui {
    NetClient::NetClient(std::string udpServer, int port) {
        this->udpServer = udpServer;
        this->port = port;
        this->receiveQueue = std::make_shared<sdrcomms::ReceiveQueue>(sdrcomms::DEFAULT_QUEUE_SIZE,
                                                                      sdrcomms::REQUIRED_SAMPLES);
        this->lastHeartbeat = std::chrono::steady_clock::now();
    }


    /**
     * accessor for the receive queue
     * @return
     */
    std::shared_ptr<sdrcomms::ReceiveQueue> NetClient::getReceiveQueue() {
        return this->receiveQueue;
    }

    /**
     * open a connection to the udp server
     * @return
     */
    bool NetClient::connect() {
        // already connected
        if (isConnected.load(std::memory_order_acquire)) {
            return true;
        }
        // create the socket handle and server address struct.
        // this is connectionless so we are not exactly connecting to the server
        // in this case, just storing a handle and address.
        server_socket = socket(AF_INET, SOCK_DGRAM, 0);
        std::memset(&server_address, 0, sizeof(server_address));
        server_address.sin_family = AF_INET;
        server_address.sin_port = htons(port);
        inet_pton(AF_INET, udpServer.c_str(), &server_address.sin_addr);



        return true;
    }

    /**
     * subscribe to the udp server to receive samples
     * this will cause samples to start streaming into the
     * receiveQueue.
     * @return
     */
    bool NetClient::subscribe() {
        // alread subscribed.
        if (isSubscribed.load(std::memory_order_relaxed)) {
            return true;
        }
        if (isConnected.load(std::memory_order_acquire)) {
            this->send_msg("REG");
            isSubscribed.store(true, std::memory_order_release);

            // setup the threads
            receive_thread = std::thread([this]() {
               this->receive_stream();
            });
            heartbeat_thread = std::thread([this]() {
               this->heartbeat();
            });

            return true;
        }
        return false;
    }

    /**
     * unsubscribe from the udp server.
     * this will stop samples streaming into the receive queue.
     * @return
     */
    bool NetClient::unsubscribe() {
        if (isConnected.load(std::memory_order_acquire)) {
            this->send_msg("UNREG");
            isSubscribed.store(false, std::memory_order_release);
            return true;
        }
        return true;
    }

    /**
     * disconnect from the server.
     * @return
     */
    bool NetClient::disconnect() {
        if (isConnected.load(std::memory_order_relaxed)) {
            if (isSubscribed.load(std::memory_order_acquire)) {
                this->unsubscribe();
            }
            // let the threads join
            if (receive_thread.joinable()) {
                receive_thread.join();
            }
            if (heartbeat_thread.joinable()) {
                heartbeat_thread.join();
            }
            // close the socket.
            if (server_socket != 0) {
                close(server_socket);
            }
        }
        // already disconnected
        return true;
    }

    /**
         * send a control message
         */
    void NetClient::send_msg(std::string msg) {
        const char* data = msg.c_str();
        sendto(server_socket,
            data,
            msg.size(),
            0,
            (sockaddr*)&server_address,
            sizeof(server_address));
    }


    /**
         * receive samples from the udp server
         * @param server_socket
         */
    void NetClient::receive_stream() {
        while (isSubscribed.load(std::memory_order_relaxed)) {
            // TODO: need to read data from the stream
            // need to buffer the data up to expected number of samples
            // then binary deserialize this into the payload structure
            // and convert into IQBlock data type.
            // then put into the receive queue.

        }
    }

    /**
     * send heartbeats to the udp server.
     * this repeats the register request.
     * @param server_socket
     */
    void NetClient::heartbeat() {
        while (isSubscribed.load(std::memory_order_relaxed)) {
            auto now = std::chrono::steady_clock::now();
            long delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->lastHeartbeat).count();
            if (delta > sdrcomms::HEARTBEAT_MS) {
                // send a heart beat.
                this->send_msg("REG");
                this->lastHeartbeat = now;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
} // sdrcommsui
