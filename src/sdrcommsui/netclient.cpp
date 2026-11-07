//
// Created by Chris Davey on 29/9/2026.
//

#include "include/netclient.h"

#include <algorithm>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#include "ChunkHeader.h"
#include "constants.h"
#include "NetIQBlock.h"

namespace sdrcommsui {
    NetClient::NetClient(std::string udpServer, int port, size_t sample_size) {
        this->udpServer = udpServer;
        this->port = port;
        this->sample_size = sample_size;
        this->receiveQueue = std::make_shared<sdrcomms::ReceiveQueue>(sdrcomms::DEFAULT_QUEUE_SIZE,
                                                                      sample_size);
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
    bool NetClient::connect_server() {
        // already connected
        if (isConnected.load(std::memory_order_relaxed)) {
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
        if (connect(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) >= 0) {
            isConnected.store(true, std::memory_order_relaxed);
            return true;
        }
        return false;
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
                isSubscribed.store(false, std::memory_order_release);
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
            isConnected.store(false, std::memory_order_release);
        }
        // already disconnected
        return true;
    }

    /**
         * send a control message
         */
    void NetClient::send_msg(std::string msg) {
        const char *data = msg.c_str();
        size_t sent = sendto(server_socket,
                             data,
                             msg.size(),
                             0, // MSG_WAITALL
                             (sockaddr *) &server_address,
                             sizeof(server_address));
        if (sent < 0) {
            std::cerr << "Error sending message: " << strerror(errno) << std::endl;
        }
    }


    /**
         * receive samples from the udp server
         * @param server_socket
         */
    void NetClient::receive_stream() {
        // chunk buffer
        size_t chunk_header_size = sdrcomms::ChunkHeader::getBytesize();
        std::vector<uint8_t> chunk_header_buffer(chunk_header_size);
        size_t total_bytes_read = 0;
        bool start_frame = false;
        // collect received frames to reorder.
        std::vector<FrameChunk> frame_chunks{};
        while (isSubscribed.load(std::memory_order_relaxed)) {
            // we need to collect data into a frame that will eventually be parsed into a chunk.
            // the first thing we need to do is read the chunk_header and parse it.
            ssize_t bytes_read = recvfrom(server_socket,
                                          chunk_header_buffer.data(),
                                          chunk_header_buffer.size(),
                                          MSG_WAITALL, //0,
                                          nullptr,
                                          nullptr);
            sdrcomms::ChunkHeader chunk_header{};
            if (bytes_read > 0 && bytes_read == chunk_header_size) {
                try {
                    // we deserialize the chunk header.
                    VectorReadBuffer chunkBuffer(chunk_header_buffer);
                    std::istream chunkStream(&chunkBuffer);
                    chunk_header = sdrcomms::ChunkHeader::deserialize(chunkStream);
                } catch (std::exception &ex) {
                    continue;
                }
            } else {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
                continue;
            }
            if (chunk_header.chunk_index == 0) {
                total_bytes_read = 0;
                frame_chunks.clear();
                start_frame = true;
            }
            if (!start_frame && chunk_header.chunk_index > 0) {
                continue;
            }
            // if we have a chunk header we can consume the remaining bytes
            // the expected bytes we need to read the header.
            size_t expected_bytes = chunk_header.getPayloadByteSize();
            // read buffer
            std::vector<uint8_t> buffer(expected_bytes);
            bytes_read = recvfrom(server_socket,
                                  buffer.data(),
                                  buffer.size(),
                                  MSG_WAITALL, //0,
                                  nullptr,
                                  nullptr);
            if (bytes_read > 0) {
                std::vector<uint8_t> frame{};
                frame.insert(frame.end(), buffer.begin(), buffer.begin() + bytes_read);
                FrameChunk chunk;
                chunk.chunk_index = chunk_header.chunk_index;
                chunk.total_chunks = chunk_header.chunk_count;
                chunk.data = std::move(frame);
                frame_chunks.push_back(std::move(chunk));
                total_bytes_read += bytes_read;
            } else {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }
            // in the future we may need to reorder chunks based on index.
            // have we read the last expected chunk?
            if (chunk_header.chunk_index == chunk_header.chunk_count - 1) {
                try {
                    // we need to deserialize the frame.
                    // First order all frame chunks by index.
                    std::ranges::sort(frame_chunks, [](const FrameChunk& a, const FrameChunk& b) {
                        return a.chunk_index < b.chunk_index;
                    });
                    // now we want to create the full frame from the received chunks.
                    std::vector<uint8_t> frame;
                    for (const auto& chunk : frame_chunks) {
                        frame.insert(frame.end(), chunk.data.begin(), chunk.data.end());
                    }
                    // deserialize the frame.
                    VectorReadBuffer frameBuffer(frame);
                    std::istream frameStream(&frameBuffer);
                    sdrcomms::NetIQBlock block = sdrcomms::NetIQBlock::deserialize(frameStream);
                    sdrcomms::IQBlock iqBlock = block.toIQBlock();
                    if (block.timestampMs != 0 && iqBlock.samples.size() > 0) {
                        this->receiveQueue->push(iqBlock.timestampMs,
                                                 iqBlock.channel,
                                                 iqBlock.samples,
                                                 iqBlock.samples.size(),
                                                 iqBlock.centerFreq);
                        auto now = std::chrono::system_clock::now();
                        auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->last_packet).count();
                        if (delta >= ONE_SECOND) {
                            last_packet = now;
                            std::cout << "Blocks per second: " << this->blocks_per_second << std::endl;
                            this->blocks_per_second = 0;
                        }
                        this->blocks_per_second++;
                    }
                } catch (std::exception &ex) {
                    //
                }
            }
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
