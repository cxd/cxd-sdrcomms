//
// Created by Chris Davey on 20/9/2026.
//

#include "include/client.h"
#include "SoapySDR/Types.hpp"
#include <iostream>
#include <thread>

namespace sdrcomms {
    Client::Client(std::string serverUrl,
                   std::string driverName,
                   std::string remoteDriverName,
                   std::string remoteDeviceUri,
                   std::string remoteBindAddr,
                   size_t queueDepth) {
        this->serverUrl = serverUrl;
        this->driverName = driverName;
        this->queueDepth = queueDepth;
        this->remoteDriverName = remoteDriverName;
        this->remoteDeviceUri = remoteDeviceUri;
        this->remoteBindAddr = remoteBindAddr;
    }

    bool Client::Connect() {
        SoapySDR::Kwargs args;
        args["device"] = driverName;
        args["remote"] = serverUrl;
        if (remoteBindAddr.length() > 0) {
            args["remote:bind"] = remoteBindAddr;
        }
        if (remoteDriverName.length() > 0) {
            args["remote:driver"] = remoteDriverName;
        }
        // customisation for plutosdr.
        if (driverName == "plutosdr") {
            // custom configuration
            std::string mtuStr = std::to_string(DEFAULT_MTU);
            args["buffer_size"] = mtuStr;
        }
        // this can be supplied as a command line argument.
        // use:
        // SoapySDRUtil --find="driver=remote,remote=tcp://localhost:11111"
        // to find device details
        if (remoteDeviceUri.length() > 0) {
            args["remote:uri"] = remoteDeviceUri;
            args["remote:no-log"] = "true";
        }

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
#ifdef CL_DEBUG_PLUTO
        std::cout << "Set Center Frequency: " << frequency << std::endl;
#endif
        centerFrequency.store(frequency, std::memory_order_relaxed);
        for (int i = 0; i < numChannels; i++) {
            device->setFrequency(SOAPY_SDR_RX, i, frequency);
        }
        dropBlocks.store(10, std::memory_order_relaxed);
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
            device->setBandwidth(SOAPY_SDR_RX, i, sampleRate);
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
        streamArgs["mtu"] = mtuStr;

        if (serverUrl.length() > 0) {
            streamArgs["remote:mtu"] = mtuStr; // Limit packet size to standard safe transmission sizes
            streamArgs["remote:format"] = "CS16";
            streamArgs["remote:window"] = winStr; // Force a sane, small socket ring buffer depth!
            streamArgs["remote:timeout"] = "200"; // milliseconds timeout
            streamArgs["remote:buffer_size"] = mtuStr;
        } else {
            streamArgs["buffer_size"] = mtuStr;
        }
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
        std::cout << "Stream MTU: " << mtu << std::endl;
        receiveQueues.resize(numChannels);
        for (int i = 0; i < numChannels; i++) {
            // samples per second
            auto queue = std::make_shared<ReceiveQueue>(queueDepth, REQUIRED_SAMPLES);
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
        // a vector to hold the previous samples that were not read.
        std::vector<std::complex<float>> prev {};
        while (isStreaming.load(std::memory_order_relaxed)) {
            size_t samples_read = 0;
            size_t src_pos = 0;
            std::vector<std::complex<float>> dest {};
            // if we were left with any previous data we need to copy it into the destination.
            if (prev.size() > 0) {
                samples_read = accumulate_samples(REQUIRED_SAMPLES, samples_read, 0, prev, dest);
                prev.clear();
            }
            while (samples_read < REQUIRED_SAMPLES && isStreaming.load(std::memory_order_relaxed)) {
                // vector to read into
                std::vector<std::complex<float>> samples(mtu);
                void *buffer[] = {samples.data()};
                // we now need to read
                int numsamples = device->readStream(stream, buffer, mtu, flags, timeNs, timeoutUs);
                if (numsamples < 0) {
                    std::cerr << "[" << channel << "] Failed to read stream data." << std::endl;
                    continue;
                }

                if (dest.size() + samples.size() > REQUIRED_SAMPLES) {
                    // we need to only take enough samples
                    std::vector<std::complex<float>> tmp;
                    prev.clear();
                    tmp.reserve(REQUIRED_SAMPLES - dest.size());
                    std::copy(samples.begin(), samples.begin() + (REQUIRED_SAMPLES - dest.size()), std::back_inserter(tmp));
                    samples_read = accumulate_samples(REQUIRED_SAMPLES, samples_read, src_pos, tmp, dest);
                    // slice the remaining samples into the previous vector.
                    size_t n = samples.size() - tmp.size();
                    std::copy(samples.begin() + n, samples.end(), std::back_inserter(prev));

                } else {
                    samples_read = accumulate_samples(REQUIRED_SAMPLES, samples_read, src_pos, samples, dest);

                }

            }

            double freq = centerFrequency.load(std::memory_order_relaxed);
            if (samples_read >= REQUIRED_SAMPLES) {

#ifdef CL_DEBUG_PLUTO
                std::cout << "[" << channel << "] Read: " << samples_read << " " << mtu << std::endl;
#endif
                std::cout << "[" << channel << "] Read: " << samples_read << " " << mtu << std::endl;
                int attempt = 0;
                // try multiple times to push the data allowing the queue to catch up.
                auto now = std::chrono::system_clock::now();
                auto duration = now.time_since_epoch();
                auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
                long long timestampMs = millis.count();
                while (!queue->push(timestampMs, channel, dest, dest.size(), freq) && attempt < 5) {
                    std::this_thread::yield();
                    ++attempt;
                }
                // reset samples read
                samples_read = 0;
                dest.clear();
                if (attempt > 5) {
                    std::cerr << "[" << channel << "] Failed to push data to queue after multiple attempts." <<
                            std::endl;
                }
            }
        }
    }


    /**
     * accumulate the samples from the last count of samples
     * to the new count of samples.
     * write from the source buffer into the destination.
     * @param source
     * @param destination
     * @return
     * the number of samples copied from the source into the destination.
     */
    size_t Client::accumulate_samples(size_t numSamples, size_t dest_pos, size_t src_pos, std::vector<std::complex<float>>& source, std::vector<std::complex<float>>& destination) {
        if (dest_pos >= numSamples) {
            return 0;
        }
        if (src_pos > source.size()) {
            return 0;
        }

        size_t available_samples = source.size() - src_pos;
        size_t safe_n = std::min(numSamples, available_samples);

        size_t delta = dest_pos + safe_n;
        if (delta > destination.size()) {
            destination.resize(delta);
        }

        std::copy(source.begin() + src_pos,
            source.begin() + src_pos + safe_n,
            destination.begin() + dest_pos);
        // in
        return destination.size();
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
                        std::cout << "[" << channel << "] Process block: " << block.samples.size() << std::endl;
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
