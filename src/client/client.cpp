//
// Created by Chris Davey on 20/9/2026.
//

#include "include/client.h"
#include "SoapySDR/Types.hpp"
#include <iostream>
#include <thread>

namespace sdrcomms {
    Client::Client(std::string serverUrl, std::string driverName, size_t queueDepth) {
        this->serverUrl = serverUrl;
        this->driverName = driverName;
        this->queueDepth = queueDepth;
    }

    bool Client::Connect() {
        SoapySDR::Kwargs args;
        args["driver"] = driverName;
        args["remote"] = serverUrl;
        args["remote:no-log"] = "true";
        // TODO: will need to make this a parameter.
        // this can be supplied as a command line argument.
        // use:
        // SoapySDRUtil --find="driver=remote,remote=tcp://localhost:11111"
        // to find device details
        args["remote:bind"] = "192.168.0.102";
        args["remote:driver"] = "plutosdr";
        args["remote:uri"] = "ip:pluto.local";

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
            numChannels = numRxChannels;

            std::cout << "Device Driver: " << deviceDriver << std::endl;
            std::cout << "Number of RX Channels: " << numRxChannels << std::endl;

            for (int i = 0; i < numRxChannels; i++) {
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
                    if (minFrequency == 0) {
                        minFrequency = item->minimum();
                    }
                    maxFrequency = item->maximum();
                    std::cout << item->minimum() << " to " << item->maximum() << std::endl;
                }
                std::cout << "Supported Sample Rates Hz" << std::endl;
                for (auto item = sampleRateRange.begin(); item != sampleRateRange.end(); ++item) {
                    if (minSampleRate == 0) {
                        minSampleRate = item->minimum();
                    }
                    maxSampleRate = item->maximum();
                    std::cout << item->minimum() << " to " << item->maximum() << std::endl;
                }
                std::cout << "Support Bandwidths" << std::endl;
                for (auto item = bandwidthRange.begin(); item != bandwidthRange.end(); ++item) {
                    if (minBandwidth == 0) {
                        minBandwidth = item->minimum();
                    }
                    maxBandwidth = item->maximum();
                    std::cout << item->minimum() << " to " << item->maximum() << std::endl;
                }
            }
            std::vector<std::string> sensors = device->listSensors();
            std::cout << "Supported Sensors" << std::endl;
            for (auto sensor: sensors) {
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


    bool Client::StreamData(std::shared_ptr<SampleProcessor> processor) {
        if (isStreaming.load(std::memory_order_relaxed)) {
            return false;
        }
        if (activeStream != nullptr) {
            // stream is active and not terminated
            return false;
        }

        if (device == nullptr || !isConnected) {
            // not initialised
            return false;
        }
        // setup processing threads internally.
        bool started = this->beginStream(processor);
        return started;
    }

    void Client::SetCenterFrequency(double frequency) {
        if (frequency < minFrequency || frequency > maxFrequency) {
            return;
        }
        if (!isConnected) {
            return;
        }
        centerFrequency.store(frequency, std::memory_order_relaxed);
        for (int i = 0; i < numChannels; i++) {
            device->setFrequency(SOAPY_SDR_RX, i, frequency);
        }
        dropBlocks.store(2, std::memory_order_relaxed);
    }


    void Client::SetSampleRateMSps(double sampleRate) {
        if (sampleRate < minSampleRate || sampleRate > maxSampleRate) {
            return;
        }
        if (!isConnected) {
            return;
        }
        for (int i = 0; i < numChannels; i++) {
            device->setSampleRate(SOAPY_SDR_RX, i, sampleRate);
        }
        dropBlocks.store(2, std::memory_order_relaxed);
        this->sampleRate.store(sampleRate, std::memory_order_relaxed);
        this->bandwidth.store(sampleRate, std::memory_order_relaxed);
    }

    bool Client::StopStreaming() {
        if (device == nullptr || !isConnected) {
            return false;
        }
        if (activeStream == nullptr) {
            return false;
        }
        isStreaming.store(false, std::memory_order_relaxed);
        // now we iterate each of the active threads
        for (auto &t: receiverThreads) {
            if (t.joinable()) {
                t.join();
            }
        }
        for (auto &t: processorThreads) {
            if (t.joinable()) {
                t.join();
            }
        }
        // clean up the stream
        device->deactivateStream(activeStream);
        device->closeStream(activeStream);
        activeStream = nullptr;
        return true;
    }

    bool Client::beginStream(std::shared_ptr<SampleProcessor> processor) {
        std::vector<size_t> channels;
        // let the driver assign the channels
        /**
         *channels.resize(numChannels);
        for (size_t i = 0; i < numChannels; i++) {
            channels.push_back(i);
        }
        **/
        SoapySDR::Kwargs streamArgs;
        std::string mtuStr = std::to_string(DEFAULT_MTU);
        std::string winStr = std::to_string(DEFAULT_WINDOW_SIZE);
        streamArgs["remote:mtu"] = mtuStr; // Limit packet size to standard safe transmission sizes
        streamArgs["remote:window"] = winStr; // Force a sane, small socket ring buffer depth!

        activeStream = device->setupStream(SOAPY_SDR_RX, streamFormat, channels, streamArgs);


        if (!activeStream) {
            isStreaming.store(false, std::memory_order_relaxed);
            return false;
        }
        // setup streaming flag
        isStreaming.store(true, std::memory_order_relaxed);

        int flags = 0;
        long long timeNs = 0;
        size_t numElems = 0;
        int result = device->activateStream(activeStream, flags, timeNs, numElems);
        if (result != 0) {
            std::cerr << "Error activating stream: " << result << std::endl;
            return false;
        }
        // sleep before setting up threads
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        size_t mtu = device->getStreamMTU(activeStream);
        receiveQueues.resize(numChannels);
        for (int i = 0; i < numChannels; i++) {
            // samples per second
            auto queue = std::make_shared<ReceiveQueue>(queueDepth, mtu);
            receiveQueues[i] = queue;
        }

        // setup receiver threads.
        for (int i = 0; i < numChannels; i++) {
            auto queuePtr = receiveQueues[i];
            receiverThreads.push_back(std::thread(
                    [this, i, queuePtr, mtu]() {
                        this->readLoop(i, this->activeStream, queuePtr, mtu);
                    })
            );
        }
        // setup processor thread

        for (int i = 0; i < numChannels; i++) {
            auto queuePtr = receiveQueues[i];
            processorThreads.push_back(std::thread(
                [this, i, queuePtr, mtu, processor]() {
                    this->processLoop(i, queuePtr, mtu, processor);
                }
            ));
        }


        return true;
    }

    void Client::readLoop(int channel, SoapySDR::Stream *stream, std::shared_ptr<ReceiveQueue> queue, size_t mtu) {
        int flags = 0;
        long long timeNs = 0;
        const long timeoutUs = 100000;
        while (isStreaming.load(std::memory_order_relaxed)) {
            IQBlock block{mtu};
            void *buffer[] = {block.samples.data()};
            int numsamples = device->readStream(stream, buffer, mtu, flags, timeNs, timeoutUs);
            if (numsamples < 0) {
                std::cerr << "[" << channel << "] Failed to read stream data." << std::endl;
            }
            double freq = centerFrequency.load(std::memory_order_relaxed);
            if (numsamples > 0) {
#ifdef CL_DEBUG_PLUTO
                std::cout << "[" << channel << "] Read: " << numsamples << " " << mtu << std::endl;
#endif
                std::cout << "[" << channel << "] Read: " << numsamples << " " << mtu << std::endl;
                int attempt = 0;
                // try multiple times to push the data allowing the queue to catch up.
                while (!queue->push(block.samples.data(), numsamples, freq) && attempt < 5) {
                    std::this_thread::yield();
                    ++attempt;
                }
                if (attempt > 5) {
                    std::cerr << "[" << channel << "] Failed to push data to queue after multiple attempts." <<
                            std::endl;
                }
            }
        }
    }

    /**
     * enter the process loop
     */
    void Client::processLoop(int channel, std::shared_ptr<ReceiveQueue> queue, size_t mtu,
                             std::shared_ptr<SampleProcessor> processor) {
        while (isStreaming.load(std::memory_order_relaxed)) {
            IQBlock block{mtu};
            if (queue->pop(block)) {
                bool process_data = false;
                if (dropBlocks.load(std::memory_order_relaxed) <= 0) {
                    process_data = true;
                } else {
                    // subtract 1
                    dropBlocks.fetch_sub(1, std::memory_order_relaxed);
                }
                if (process_data) {
                    try {
                        std::cerr << "[" << channel << "] Process block: " << block.samples.size() << std::endl;
                        // execute the callback
                        processor->process(block);
                    } catch (std::exception &e) {
                        std::cerr << "[" << channel << "] Exception in callback: " << e.what() << std::endl;
                    }
                } else {
                    std::this_thread::yield();
                }
            } else {
                std::this_thread::yield();
            }
        }
    }
}
