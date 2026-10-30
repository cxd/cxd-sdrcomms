//
// Created by Chris Davey on 11/10/2026.
//

#include "MQTTServer.h"

#include <iostream>

#include "constants.h"

namespace sdrcomms {
    MQTTServer::MQTTServer(std::string serverAddress, int port,
                           std::shared_ptr<MessageQueue<ControlMessage> > sendToClientQueue) {
        this->serverAddress = serverAddress;
        this->port = port;
        this->messageQueue = std::make_shared<MessageQueue<ControlMessage> >(DEFAULT_QUEUE_SIZE);
        this->sendToClientQueue = sendToClientQueue;
    }

    /**
         * get the message queue for the MQTT communication.
         * @return
         */
    std::shared_ptr<MessageQueue<ControlMessage> > MQTTServer::getMessageQueue() {
        return this->messageQueue;
    }

    bool MQTTServer::init() {
        int result = mosquitto_lib_init();
        if (result != MOSQ_ERR_SUCCESS) {
            return false;
        }
        bool clean_session = true;
        this->mServer = mosquitto_new("sdrcomms::MQTTServer",
                                      clean_session,
                                      this); // construct with this to pass through
        if (this->mServer == nullptr) {
            mosquitto_lib_cleanup();
            return false;
        }
        auto connect_callback = [](struct mosquitto *mosq, void *obj, int result) {
            auto *self = static_cast<MQTTServer *>(obj);
            self->on_connect(mosq, obj, result);
        };
        auto publish_callback = [](struct mosquitto *mosq, void *obj, int mid) {
            auto *self = static_cast<MQTTServer *>(obj);
            self->on_publish(mosq, obj, mid);
        };
        auto message_callback = [](struct mosquitto *mosq, void *obj, const struct mosquitto_message *message) {
            auto *self = static_cast<MQTTServer *>(obj);
            self->on_message(mosq, obj, message);
        };
        mosquitto_connect_callback_set(this->mServer, connect_callback);
        mosquitto_publish_callback_set(this->mServer, publish_callback);
        mosquitto_message_callback_set(this->mServer, message_callback);

        workerThread = std::thread([this]() {
            this->incoming_message_thread();
        });

        isInit.store(true, std::memory_order_relaxed);
        return true;
    }

    bool MQTTServer::start() {
        if (!isInit.load(std::memory_order_relaxed)) {
            return false;
        }
        if (running.load(std::memory_order_relaxed)) {
            return true;
        }
        int result = mosquitto_loop_start(this->mServer);
        if (result != MOSQ_ERR_SUCCESS) {
            mosquitto_destroy(this->mServer);
            mosquitto_lib_cleanup();
            isInit.store(false, std::memory_order_relaxed);
            return false;
        }
        // now we need to connect to the local server process.
        result = mosquitto_connect(this->mServer,
                                   serverAddress.c_str(),
                                   port,
                                   keepAlive);
        if (result != MOSQ_ERR_SUCCESS) {
            std::cerr << "Failed to connect to MQTT server: " << serverAddress << ":" << port << std::endl;
            mosquitto_destroy(this->mServer);
            mosquitto_lib_cleanup();
            isInit.store(false, std::memory_order_relaxed);
            return false;
        }
        running.store(true, std::memory_order_relaxed);
        return true;
    }

    bool MQTTServer::stop() {
        if (!isInit.load(std::memory_order_relaxed)) {
            return false;
        }
        if (!running.load(std::memory_order_relaxed)) {
            return true;
        }
        if (workerThread.joinable()) {
            workerThread.join();
        }
        mosquitto_disconnect(this->mServer);
        mosquitto_loop_stop(this->mServer, true);
        mosquitto_destroy(this->mServer);
        mosquitto_lib_cleanup();
        running.store(false, std::memory_order_relaxed);
        isInit.store(false, std::memory_order_relaxed);
        return true;
    }

    /**
         * handle connections to the server process
         * @param mosq
         * @param obj
         * @param result
         */
    void MQTTServer::on_connect(struct mosquitto *mosq, void *obj, int result) {
        // successful connection to the server we can begin processing messages.
    }

    /**
     * after message is published
     * @param mosq
     * @param obj
     * @param mid
     */
    void MQTTServer::on_publish(struct mosquitto *mosq, void *obj, int mid) {
        //  we don't do anything with this.
    }

    /**
     * receive messages
     * @param mosq
     * @param obj
     * @param message
     */
    void MQTTServer::on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message) {
        if (!running.load(std::memory_order_relaxed)) {
            return;
        }
        // convert the message into the message type and
        // write the message into the queue.
        ControlMessage msg;
        if (!ControlMessage::parseMQTTMessage(msg, message)) {
            std::this_thread::yield();
            return;
        }
        // successfully read message
        messageQueue->push(msg);
    }

    void MQTTServer::send_control_command(const std::string& message_topic, const ControlMessage& msg) {

        const std::vector<uint8_t>& msg_cbor = msg.toCBOR();
        // Compress down to highly optimized binary CBOR vector
        std::string target_topic = "sdrcomms/" + message_topic + "/control";

        mosquitto_publish(this->mServer, nullptr, target_topic.c_str(),
                          msg_cbor.size(), msg_cbor.data(),
                          1, false); // Using QoS 1 to guarantee command delivery
    }


    /**
         * a thread to read control messages from the incoming queue and send to clients.
         */
    void MQTTServer::incoming_message_thread() {
        while (running.load(std::memory_order_relaxed)) {
            ControlMessage controlMessage{};
            if (sendToClientQueue->pop(controlMessage)) {
                // we want to publish the message to a topic.
                std::string topic = controlMessage.message_type_to_string();
                send_control_command(topic, controlMessage);
            }
            std::this_thread::sleep_for(std::chrono::microseconds(200));
        }

    }
} // sdrcomms
