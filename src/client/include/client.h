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
#include <thread>

#include "IQBlock.h"
#include "ReceiveQueue.h"
#include "constants.h"
#include "SampleProcessor.h"

namespace sdrcomms {
    class Client {
    public:
        /**
         *
         * @param serverUrl the remote server address tcp://ipaddr:port
         * @param driverName the name of the driver "remote" for soapy server.
         * @param remoteDriverName the name of the device driver managed by the remote server eg: "putosdr"
         * @param remoteDeviceUri the remote device uri eg: "tcp:pluto.local"
         * @param remoteBindAddr the bind address for communications with the server this would be the ipaddr of the server
         * @param queueDepth the depth of the queue used to store messages.
         */
        Client(std::string serverUrl,
               std::string driverName,
               std::string remoteDriverName,
               std::string remoteDeviceUri,
               std::string remoteBindAddr,
               size_t queueDepth,
               size_t required_samples);

        bool Connect();

        void PrintSoapySDRInfo();

        void PrintDeviceInfo();

        /**
         * start streaming
         * @param callback
         * @return
         * true on success false otherwise.
         */
        bool StreamData(std::shared_ptr<SampleProcessor> processor);



        /**
         * set the frequency for the device
         * must be connected
         * @param frequency
         */
        void SetCenterFrequency(double frequency);

        /**
         * return the current center frequency
         * @return
         */
        double GetCenterFrequency();

        /**
         * set the sample rate for the device
         * must be connected
         * @param sampleRate
         */
        void SetSampleRateMSps(double sampleRate);

        /**
         * set the default receiver gain
         * @param gain
         * @return
         */
        void SetRxGain(double gain);

        /**
         * get the rx gain frm the device
         * @return
         */
        double GetRxGain();

        /**
         * set the transmit gain to 0 to reduce possible interference
         * with the receiver when using the device for listening only
         */
        void DisableTransmitter();

        bool StopStreaming();

    private:
        std::string serverUrl;
        std::string driverName;
        std::string remoteDriverName;
        std::string remoteDeviceUri;
        std::string remoteBindAddr;
        bool isConnected;

        /**
         * number of blocks to store per channel in the ring buffer
         */
        size_t queueDepth = DEFAULT_QUEUE_SIZE;
        std::string streamFormat = "CF32";

        std::shared_ptr<SoapySDR::Device> device;
        /**
         * receiver queues one for each channel.
         */
        std::vector<std::shared_ptr<ReceiveQueue>> receiveQueues;
        // internal flag indicating if streaming is active.
        std::atomic<bool> isStreaming;
        // current scan frequency
        std::atomic<double> centerFrequency;
        // sample rate
        std::atomic<double> sampleRate;
        // bandwidth
        std::atomic<double> bandwidth;

        // after changing hardware configuration eg frequency, we should discard this many blocks
        std::atomic<int> dropBlocks{10 };

        /**
         * default sample rate for streaming
         */
        double defaultSampleRateMsps = DEFAULT_SAMPLE_RATE_HZ;
        double defaultBandwidthHz = DEFAULT_SAMPLE_RATE_HZ;
        /**
         * default number of samples to read
         **/
        size_t required_samples = REQUIRED_SAMPLES;

        double rxGain = RX_DEFAULT_GAIN_DB;

        /**
         * configuration from the device.
         */
        double minSampleRate, maxSampleRate;
        double minBandwidth, maxBandwidth;
        double minFrequency, maxFrequency;
        double minGain, maxGain;

        int numChannels;

        /**
         * internal threads
         **/
        std::vector<std::thread> receiverThreads{};
        std::vector<std::thread> processorThreads{};

        // active stream pointer
        SoapySDR::Stream* activeStream = nullptr;


        /**
         * internal thread to begin the streaming over all channels
         * and wait for the threads to join.
         */
        bool beginStream(std::shared_ptr<SampleProcessor> processor);

        /**
         * internal read loop to perform reading of IQ data from the device
         */
        void readLoop(int channel, SoapySDR::Stream* stream, std::shared_ptr<ReceiveQueue> queue, size_t mtu);

        /**
         * accumulate the samples from the last count of samples
         * to the new count of samples.
         * write from the source buffer into the destination.
         * @param source
         * @param destination
         * @return
         */
        size_t accumulate_samples(size_t numSamples, size_t dest_pos, size_t src_pos, std::vector<std::complex<float>>& source, std::vector<std::complex<float>>& destination) ;

        /**
         * internal processing loop to perform processing of received IQ data
         * and dispatch on the callback.
         */
        void processLoop(int channel, std::shared_ptr<ReceiveQueue> queue, size_t mtu, std::shared_ptr<SampleProcessor> processor);
    };
}



#endif //SDRCOMMS_CLIENT_H
