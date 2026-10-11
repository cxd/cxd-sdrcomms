//
// Created by Chris Davey on 11/10/2026.
//

#ifndef SDRCOMMS_MQTTSERVER_H
#define SDRCOMMS_MQTTSERVER_H
#include<thread>
#include<atomic>
#include <memory>

#include "ControlMessage.h"
#include "MessageQueue.h"
#include "mosquitto.h"
namespace sdrcomms {
    class MQTTServer {
    public:
        MQTTServer(std::string serverAddress, int port);

        bool init();
        bool start();
        bool stop();

        /**
         * get the message queue for the MQTT communication.
         * @return
         */
        std::shared_ptr<MessageQueue<ControlMessage>> getMessageQueue();
    private:
        std::string serverAddress;
        int port;
        int keepAlive = 60;
        std::atomic<bool> running;
        std::atomic<bool> isInit;
        std::thread serverThread;

        mosquitto* mServer;

        std::shared_ptr<MessageQueue<ControlMessage>> messageQueue;

        /**
         * handle connections to the server process
         * @param mosq
         * @param obj
         * @param result
         */
        void on_connect(struct mosquitto* mosq, void* obj, int result);

        /**
         * after message is published
         * @param mosq
         * @param obj
         * @param mid
         */
        void on_publish(struct mosquitto* mosq, void* obj, int mid);

        /**
         * receive messages
         * @param mosq
         * @param obj
         * @param message
         */
        void on_message(struct mosquitto* mosq, void* obj, const struct mosquitto_message* message);
    };
} // sdrcomms

#endif //SDRCOMMS_MQTTSERVER_H
