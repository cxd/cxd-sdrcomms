//
// Created by Chris Davey on 20/9/2026.
//

#include "include/client.h"
#include "SoapySDR/Types.hpp"
#include <iostream>

namespace sdrcomms {
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
            SoapySDR::Device *rawDevice = SoapySDR::Device::make(args);
            if (rawDevice == nullptr) {
                isConnected = false;
                return false;
            }
            isConnected = true;
            // delete the device locally but we might need to store a reference
            // unless we connect and disconnect every operation.
            isConnected = true;

            device = std::shared_ptr<SoapySDR::Device>(rawDevice,
                                                       [](SoapySDR::Device *d) {
                                                           // clear memory when the
                                                           // shared pointer is removed
                                                           SoapySDR::Device::unmake(d);
                                                       }
            );

            return isConnected;
        } catch (const std::exception &E) {
            isConnected = false;
        }
        return false;
    }

    void Client::PrintDeviceInfo() {
        if (device != nullptr && isConnected) {
            std::cout << "Connection device information" << std::endl;
            std::string deviceDriver = device->getDriverKey();
            int numRxChannels = device->getNumChannels(SOAPY_SDR_RX);

            std::cout << "Device Driver: " << deviceDriver << std::endl;
            std::cout << "Number of RX Channels: " << numRxChannels << std::endl;

            for (int i=0;i<numRxChannels;i++) {
                std::cout << "Channel " << i << std::endl;
                std::vector<std::string> streamFormats = device->getStreamFormats(SOAPY_SDR_RX, i);
                double gain = device->getGain(SOAPY_SDR_RX, i);
                double freq = device->getFrequency(SOAPY_SDR_RX, i);
                SoapySDR::RangeList freqRange = device->getFrequencyRange(SOAPY_SDR_RX, i);
                double sampleRate = device->getSampleRate(SOAPY_SDR_RX, i);
                SoapySDR::RangeList sampleRateRange = device->getSampleRateRange(SOAPY_SDR_RX, i);
                double bandWidth = device->getBandwidth(SOAPY_SDR_RX, i);
                SoapySDR::RangeList bandwidthRange = device->getBandwidthRange(SOAPY_SDR_RX, i);
                std::cout << "Supported RX Channel Stream Formats" << std::endl;
                for (auto format: streamFormats) {
                    std::cout << format << std::endl;
                }
                std::cout << "Gain: " << gain << std::endl;
                std::cout << "Frequency: " << freq << std::endl;
                std::cout << "Sample Rate: " << sampleRate << std::endl;
                std::cout << "Bandwidth: " << bandWidth << std::endl;
                std::cout << "Supported Frequency Range: " << std::endl;
                for (auto item = freqRange.begin(); item != freqRange.end(); ++item) {
                    std::cout << item->minimum() << " to " << item->maximum() << std::endl;
                }
                std::cout << "Supported Sample Rates Hz" << std::endl;
                for (auto item = sampleRateRange.begin(); item != sampleRateRange.end(); ++item) {
                    std::cout << item->minimum() << " to " << item->maximum() << std::endl;
                }
                std::cout << "Support Bandwidths" << std::endl;
                for (auto item = bandwidthRange.begin(); item != bandwidthRange.end(); ++item) {
                    std::cout << item->minimum() << " to " << item->maximum() << std::endl;
                }

            }
            std::vector<std::string> sensors = device->listSensors();
            std::cout << "Supported Sensors" << std::endl;
            for (auto sensor : sensors) {
                std::cout << sensor << std::endl;
            }


        }
    }

    void Client::PrintSoapySDRInfo() {
        SoapySDR::loadModules();
        std::vector<std::string> paths = SoapySDR::listSearchPaths();
        std::cout << "Module Search Paths" << std::endl;
        for (auto path: paths) {
            std::cout << path << std::endl;
        }
        std::cout << std::endl;

        std::vector<std::string> modules = SoapySDR::listModules();
        std::cout << "Available Modules" << std::endl;
        for (auto module: modules) {
            std::cout << module << std::endl;
        }
        if (isConnected) {
            PrintDeviceInfo();
        }
    }


    void Client::ListDevices() {
    }
}
