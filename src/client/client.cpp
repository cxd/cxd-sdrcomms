//
// Created by Chris Davey on 20/9/2026.
//

#include "include/client.h"
#include "SoapySDR/Types.hpp"

namespace sdrcomms {
    // Define and initialize the static variables here!
    bool Client::isRegistered = false;
    bool Client::errState = false;

    Client::Client(std::string serverUrl, std::string driverName) {
        this->serverUrl = serverUrl;
        this->driverName = driverName;
        this->Connect();
    }

    bool Client::Connect() {

        SoapySDR::Kwargs args;
        args["driver"] = driverName;
        args["remote"] = serverUrl;
        try {
            SoapySDR::Device* device = SoapySDR::Device::make(args);
            if (device == nullptr) {
                isConnected = false;
                return false;
            }
            isConnected = true;
            // delete the device locally but we might need to store a reference
            // unless we connect and disconnect every operation.
            isConnected = true;
            SoapySDR::Device::unmake(device);
            return isConnected;
        } catch (const std::exception &E) {
            isRegistered = false;
            errState = true;
            isConnected = false;
        }
        return false;
    }


    void Client::ListDevices() {

    }


}