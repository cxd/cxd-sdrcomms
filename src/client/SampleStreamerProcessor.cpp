//
// Created by Chris Davey on 27/9/2026.
//

#include "include/SampleStreamerProcessor.h"

#include <iostream>
#include <sstream>
#include <thread>
#include <algorithm>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include "include/constants.h"
#include "include/NetIQBlock.h"

namespace sdrcomms {
    SampleStreamerProcessor::SampleStreamerProcessor(int udpPort) {
        this->udpPort = udpPort;
        this->receiveQueue = std::make_shared<ReceiveQueue>(DEFAULT_QUEUE_SIZE, REQUIRED_SAMPLES);
    }


    void SampleStreamerProcessor::process(const IQBlock &block) {
        if (running.load(std::memory_order_acquire)) {
            receiveQueue->push(block.timestampMs, block.channel, block.samples, block.samples.size(), block.centerFreq);
        }
    }


    /**
     * start the server process.
     * this is called from outside of the sdr client before passing into the client.
     */
    void SampleStreamerProcessor::startServer() {
        // initialise threads.
        // we will use a registry of clients that connect to the udp port.
        // this will ideally be a lock free structure.
        // to connect the client will need to send a register request.
        // a client will be in the list for a period of 60 seconds, if the client does not reregister
        // it will be removed
        // reregistering will look for the client and update the timeout value.
        // the stream loop will send the stream to each client.
        // a client can also send a remove request to stop receiving stream data.
        // otherwise it will be removed from the list.

        int server_socket = socket(AF_INET, SOCK_DGRAM, 0);
        if (server_socket < 0) {
            std::cerr << "Could not create server" << std::endl;
            return;
        }
        sockaddr_in server_addr{};
        std::memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(udpPort);
        if (bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "Could not bind server" << std::endl;
            close(server_socket);
            return;
        }

        // set running true
        running.store(true, std::memory_order_release);

        // now we can configure the threads.
        client_thread = std::thread(
            [this, server_socket]() {
                this->handle_client_connections(server_socket);
            }
        );

        stream_thread = std::thread(
            [this, server_socket]() {
                this->stream_samples(server_socket);
            }
        );
    }

    /**
     * stop the server process.
     */
    void SampleStreamerProcessor::stopServer() {
        running.store(false, std::memory_order_release);
        stream_thread.join();
        client_thread.join();
    }

    /**
         * wait for clients to connect
         */
    void SampleStreamerProcessor::handle_client_connections(int server_socket) {
        while (running.load(std::memory_order_acquire)) {
            // if adding a client.
            auto copy_active = std::atomic_load(&active_clients);
            if (copy_active == nullptr) {
                std::this_thread::yield();
                continue;
            }
            char recv_buffer[16];
            sockaddr_in client_addr{};
            socklen_t client_len = sizeof(client_addr);

            ssize_t recv_len = recvfrom(server_socket, recv_buffer, sizeof(recv_buffer), MSG_DONTWAIT, (sockaddr *) &client_addr,
                                        &client_len);
            if (recv_len < 0 || errno == EAGAIN || errno == EWOULDBLOCK) {
                std::this_thread::yield();
                continue;
            }
            recv_buffer[recv_len] = '\0';
            std::string command(recv_buffer);
            if (command == "REG") {
                // add a client.
                NetClient client{client_addr, std::chrono::steady_clock::now()};
                auto it = std::find(copy_active->begin(), copy_active->end(), client);
                if (it == copy_active->end()) {
                    copy_active->push_back(client);
                    std::atomic_store(&active_clients, copy_active);
                } else {
                    // existing client.
                    it->heartbeat = std::chrono::steady_clock::now();
                    std::atomic_store(&active_clients, copy_active);
                }
            } else if (command == "CLOSE" || command == "UNREG") {
                NetClient client{client_addr, std::chrono::steady_clock::now() };
                remove_client(client);
            }
            cleanup_expired_clients();
        }
        // linux or osx
        ::close(server_socket);
    }

    /**
     * iterate over each client and stream samples
     */
    void SampleStreamerProcessor::stream_samples(int server_socket) {
        while (running.load(std::memory_order_acquire)) {
            if (active_clients == nullptr || active_clients->empty()) {
                std::this_thread::yield();
                continue;
            }
            auto copy_active = std::atomic_load(&active_clients);
            IQBlock block{REQUIRED_SAMPLES};
            if (receiveQueue->pop(block)) {
                // stream data to clients.
                // serialize the block.
                NetIQBlock netBlock{block};
                std::ostringstream oss {std::ios::binary};
                if (!netBlock.serialize(oss)) {
                    // couldn't write to the stream need to skip this block.
                    continue;
                }
                std::string bytes = oss.str();

                for (auto client: *copy_active) {
                    // stream data to client.
                    // example of streaming a struct ie: IQSample. struct IQSample { int16_t i; int16_t q; };
                    // with packet buffer vector std::vector<IQSample> packet_buffer(PACKET_SAMPLES);
                    // we need a data type to hold iq values, but we also want metadata in the header.
                    //
                    sendto(server_socket, bytes.data(), bytes.size(), 0,
                        (struct sockaddr*)&client.address, sizeof(client.address));
                }
            }
        }
    }

    /**
     * cleanup expired clients.
     */
    void SampleStreamerProcessor::cleanup_expired_clients() {
        if (active_clients == nullptr || active_clients->empty()) {
            return;
        }
        std::shared_ptr<std::vector<NetClient> > new_clients{std::make_shared<std::vector<NetClient> >()};
        // find the clients that are not expired.
        auto now = std::chrono::steady_clock::now();
        auto copy_active = std::atomic_load(&active_clients);
        for (auto client: *copy_active) {
            auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - client.heartbeat).count();
            if (delta < UDP_CLIENT_STREAM_TIMEOUT * 1000) {
                new_clients->push_back(client);
            }
        }
        // atomically swap the client lists.
        std::atomic_store(&active_clients, new_clients);
    }

    void SampleStreamerProcessor::remove_client(const NetClient& client) {
        if (active_clients == nullptr || active_clients->empty()) {
            return;
        }
        auto copy_active = std::atomic_load(&active_clients);
        auto it = std::find(copy_active->begin(), copy_active->end(), client);
        if (it != copy_active->end()) {
            copy_active->erase(it);
        }
        std::atomic_store(&active_clients, copy_active);
    }
} // sdrcomms
