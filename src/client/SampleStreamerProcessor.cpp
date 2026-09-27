//
// Created by Chris Davey on 27/9/2026.
//

#include "include/SampleStreamerProcessor.h"

#include "constants.h"

namespace sdrcomms {
    SampleStreamerProcessor::SampleStreamerProcessor(int udpPort) {
        this->udpPort = udpPort;
        this->receiveQueue = std::make_shared<ReceiveQueue>(DEFAULT_QUEUE_SIZE, REQUIRED_SAMPLES);
    }


    void SampleStreamerProcessor::process(const IQBlock &block) {
        if (running.load(std::memory_order_acquire)) {
            receiveQueue->push(block.samples, block.samples.size(), block.centerFreq);
        }
    }


    /**
     * start the server process.
     * this is called from outside of the sdr client before passing into the client.
     */
    void SampleStreamerProcessor::startServer() {
        // set running true
        running.store(true, std::memory_order_release);

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
    }

    /**
     * stop the server process.
     */
    void SampleStreamerProcessor::stopServer() {
        running.store(false, std::memory_order_release);
    }

    /**
         * wait for clients to connect
         */
    void SampleStreamerProcessor::handle_client_connections() {


    }

    /**
     * iterate over each client and stream samples
     */
    void SampleStreamerProcessor::stream_samples() {
        if (active_clients == nullptr || active_clients->empty()) {
            return;
        }
        for (auto client : *active_clients) {
            // stream data to client.

        }
    }

    /**
     * cleanup expired clients.
     */
    void SampleStreamerProcessor::cleanup_expired_clients() {
        if (active_clients == nullptr || active_clients->empty()) {
            return;
        }
        std::shared_ptr<std::vector<Client> >  new_clients { std::make_shared<std::vector<Client>> ()  };
        // find the clients that are not expired.
        auto now = std::chrono::steady_clock::now();
        for (auto client : *active_clients) {
            auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - client.heartbeat).count();
            if (delta < UDP_CLIENT_STREAM_TIMEOUT * 1000) {
                new_clients->push_back(client);
            }
        }
        // atomically swap the client lists.
        std::atomic_store(&active_clients, new_clients);
    }
} // sdrcomms
