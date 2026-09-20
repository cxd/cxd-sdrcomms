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
#include <functional>
#include <atomic>
#include "IQBlock.h"
#include "ReceiveQueue.h"

namespace sdrcomms {
    class Client {
    public:
        Client(std::string serverUrl, std::string driverName);

        bool Connect();

        void PrintSoapySDRInfo();

        void PrintDeviceInfo();

        void StreamData(std::function<void(const IQBlock&)> callback);

        void SetCenterFrequency(double frequency);

        void StopStreaming();

    private:
        std::string serverUrl;
        std::string driverName;
        bool isConnected;

        // number of samples per second
        size_t samples_per_second = 2.5e6;

        std::shared_ptr<SoapySDR::Device> device;
        std::shared_ptr<ReceiveQueue> receiveQueue;
        std::atomic<bool> isStreaming;
        std::atomic<double> centerFrequency;


    };
}



#endif //SDRCOMMS_CLIENT_H
