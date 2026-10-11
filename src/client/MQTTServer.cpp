//
// Created by Chris Davey on 11/10/2026.
//

#include "MQTTServer.h"

#include "constants.h"

namespace sdrcomms {
    MQTTServer::MQTTServer(std::string serverAddress, int port) {
        this->serverAddress = serverAddress;
        this->port = port;
        this->messageQueue = std::make_shared<MessageQueue<ControlMessage>>(DEFAULT_QUEUE_SIZE);
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
        auto connect_callback = [](struct mosquitto* mosq, void* obj, int result) {
            auto* self = static_cast<MQTTServer*>(obj);
            self->on_connect(mosq, obj, result);
        };
        auto publish_callback = [](struct mosquitto* mosq, void* obj, int mid) {
            auto* self = static_cast<MQTTServer*>(obj);
            self->on_publish(mosq, obj, mid);
        };
        auto message_callback = [](struct mosquitto* mosq, void* obj, const struct mosquitto_message* message) {
            auto* self = static_cast<MQTTServer*>(obj);
            self->on_message(mosq, obj, message);
        };
        mosquitto_connect_callback_set(this->mServer, connect_callback);
        mosquitto_publish_callback_set(this->mServer, publish_callback);
        mosquitto_message_callback_set(this->mServer, message_callback);
        isInit.store(true, std::memory_order_relaxed);
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
    void MQTTServer::on_connect(struct mosquitto* mosq, void* obj, int result) {
        // successful connection to the server we can begin processing messages.

    }

    /**
     * after message is published
     * @param mosq
     * @param obj
     * @param mid
     */
    void MQTTServer::on_publish(struct mosquitto* mosq, void* obj, int mid) {
        //  we don't do anything with this.
    }

    /**
     * receive messages
     * @param mosq
     * @param obj
     * @param message
     */
    void MQTTServer::on_message(struct mosquitto* mosq, void* obj, const struct mosquitto_message* message) {
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
} // sdrcomms