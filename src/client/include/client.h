//
// Created by Chris Davey on 20/9/2026.
//

#ifndef SDRCOMMS_CLIENT_H
#define SDRCOMMS_CLIENT_H

#include "SoapySDR/Device.hpp"
#include "SoapySDR/Registry.hpp"



namespace sdrcomms {
    class Client {
    public:
        Client(std::string serverUrl, std::string driverName);

        void ListDevices();
        bool Connect();

    private:
        std::string serverUrl;
        std::string driverName;
        static bool isRegistered;
        static bool errState;
        bool isConnected;

    };
}



#endif //SDRCOMMS_CLIENT_H
