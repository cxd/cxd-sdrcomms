//
// Created by Chris Davey on 11/10/2026.
//

#ifndef SDRCOMMS_CONTROLMESSAGE_H
#define SDRCOMMS_CONTROLMESSAGE_H
#include <mosquitto.h>
#include <nlohmann/json.hpp>
using json = nlohmann::json;

namespace sdrcomms {
    enum class MessageType {
        MSG_NONE = 0,
        MSG_ACK = 1,
        MSG_NACK = 2,
        MSG_AUTO_SCAN = 3,
        MSG_STOP_SCAN = 4,
        MSG_SET_FREQ = 5,
        MSG_CLASSIFY = 6,
    };


    enum class MessageResult {
        MSG_NONE = 0,
        MSG_SUCCESS = 1,
        MSG_FAILURE = 2
    };

    struct ControlMessage {
        MessageType type = MessageType::MSG_NONE;
        MessageResult result = MessageResult::MSG_NONE;
        double freq = 0;
        std::string classLabel =  "";
        float confidence = 0;
        int channel = 0;
        /**
         * time since epoch in milliseconds.
         */
        long long timestampMs = 0;

        static bool parseJSONMessage(ControlMessage &msg, const struct mosquitto_message* message) {
            if (message->payload == nullptr) {
                return false;
            }
            try {
                const uint8_t* raw = static_cast<const uint8_t*>(message->payload);
                json j = json::parse(raw);
                msg.type = static_cast<MessageType>(j["type"].get<int>());
                msg.result = static_cast<MessageResult>(j["result"].get<int>());
                msg.freq = j["freq"].get<double>();
                msg.classLabel = j["classLabel"].get<std::string>();
                msg.confidence = j["confidence"].get<float>();
                msg.channel = j["channel"].get<int>();
                msg.timestampMs = j["timestampMs"].get<long long>();
                return true;
            }  catch (std::exception ex) {
                return false;
            }
        }

        static bool parseCBORMessage(ControlMessage &msg, const struct mosquitto_message* message) {
            if (message->payload == nullptr) {
                return false;
            }
            try {
                const uint8_t* raw = static_cast<const uint8_t*>(message->payload);
                json j = json::from_cbor(raw, raw + message->payloadlen);
                msg.type = static_cast<MessageType>(j["type"].get<int>());
                msg.result = static_cast<MessageResult>(j["result"].get<int>());
                msg.freq = j["freq"].get<double>();
                msg.classLabel = j["classLabel"].get<std::string>();
                msg.confidence = j["confidence"].get<float>();
                msg.channel = j["channel"].get<int>();
                msg.timestampMs = j["timestampMs"].get<long long>();
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
            j["classLabel"] = classLabel;
            j["confidence"] = confidence;
            j["channel"] = channel;
            j["timestampMs"] = timestampMs;
            std::vector<uint8_t> buffer = json::to_cbor(j);
            return buffer;
        }

        std::string toJSON() const {
            json j;
            j["type"] = static_cast<int>(type);
            j["result"] = static_cast<int>(result);
            j["freq"] = freq;
            j["classLabel"] = classLabel;
            j["confidence"] = confidence;
            j["channel"] = channel;
            j["timestampMs"] = timestampMs;
            return j.dump();
        }



        std::string message_type_to_string() {
            switch (this->type) {
                case MessageType::MSG_ACK:
                    return "ack";
                case MessageType::MSG_NACK:
                    return "nack";
                case MessageType::MSG_AUTO_SCAN:
                    return "auto_scan";
                case MessageType::MSG_STOP_SCAN:
                    return "stop_scan";
                case MessageType::MSG_SET_FREQ:
                    return "set_freq";
                case MessageType::MSG_CLASSIFY:
                    return "classify";
                default:
                    return "none";
            }
        }


    };
}

#endif //SDRCOMMS_CONTROLMESSAGE_H
