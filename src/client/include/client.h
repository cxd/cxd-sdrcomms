//
// Created by Chris Davey on 20/9/2026.
//

#ifndef SDRCOMMS_CLIENT_H
#define SDRCOMMS_CLIENT_H

#include "SoapySDR/Device.hpp"
#include "SoapySDR/Registry.hpp"
#include <vector>
#include "SoapySDR/Modules.hpp"
#include <memory>

namespace sdrcomms {
    class Client {
    public:
        Client(std::string serverUrl, std::string driverName);

        void ListDevices();
        bool Connect();

        void PrintSoapySDRInfo();

        void PrintDeviceInfo();

    private:
        std::string serverUrl;
        std::string driverName;
        bool isConnected;

        std::shared_ptr<SoapySDR::Device> device;
    };
}



#endif //SDRCOMMS_CLIENT_H
