//
// Created by Chris Davey on 11/10/2026.
//

#ifndef SDRCOMMS_CONTROLMESSAGE_H
#define SDRCOMMS_CONTROLMESSAGE_H
#include <mosquitto/libmosquitto.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

enum class MessageType {
    MSG_NONE = 0,
    MSG_ACK = 1,
    MSG_NACK = 2,
    MSG_AUTO_SCAN = 3,
    MSG_STOP_SCAN = 4,
    MSG_SET_FREQ = 5
};

enum class MessageResult {
    MSG_NONE = 0,
    MSG_SUCCESS = 1,
    MSG_FAILURE = 2
};

struct ControlMessage {
    MessageType type;
    MessageResult result;
    float freq;

    static bool parseMQTTMessage(ControlMessage &msg, const struct mosquitto_message* message) {
        if (message->payload == nullptr) {
            return false;
        }
        try {
            const uint8_t* raw = static_cast<const uint8_t*>(message->payload);
            json j = json::parse(raw);
            msg.type = static_cast<MessageType>(j["type"].get<int>());
            msg.result = static_cast<MessageResult>(j["result"].get<int>());
            msg.freq = j["freq"].get<float>();
            return true;
        }  catch (std::exception ex) {
            return false;
        }
    }

    std::vector<uint8_t> toCBOR() const {
        json j;
        j["type"] = static_cast<int>(type);
        j["result"] = static_cast<int>(result);
        j["freq"] = freq;
        std::vector<uint8_t> buffer = json::to_cbor(j);
        return buffer;
    }

    std::string toJSON() const {
        json j;
        j["type"] = static_cast<int>(type);
        j["result"] = static_cast<int>(result);
        j["freq"] = freq;
        return j.dump();
    }




};

#endif //SDRCOMMS_CONTROLMESSAGE_H
