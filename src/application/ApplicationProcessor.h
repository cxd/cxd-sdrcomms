//
// Created by Chris Davey on 29/9/2026.
//

#ifndef SDRCOMMS_APPLICATIONPROCESSOR_H
#define SDRCOMMS_APPLICATIONPROCESSOR_H

#include "SampleProcessor.h"
#include "IQBlock.h"
#include <chrono>

#include "FFTSampleProcessor.h"
#include "MockProcessor.h"
#include "SampleStreamerProcessor.h"

class ApplicationProcessor : public sdrcomms::SampleProcessor {
public:
    ApplicationProcessor(std::string udpServerAddress, int udpPort);

    /**
     * implement the process method
     * @param block
     */
    void process(const sdrcomms::IQBlock &block) override;

    /**
     * expose start server
     */
    void startServer();

    /**
     * expose stop server
     */
    void stopServer();
private:
    std::string udpServerAddress;
    int udpPort;
    static const long ONE_SECOND = 1000;

    std::string modelPath;
    std::string modelSpecFile;

    std::unique_ptr<MockProcessor> mockProcessor;
    std::unique_ptr<sdrcomms::SampleStreamerProcessor> sampleStreamerProcessor;
    std::unique_ptr<sdrcomms::FFTSampleProcessor> fftProcessor;

    long blocks_per_second;
    std::chrono::time_point<std::chrono::system_clock> last_packet;
};


#endif //SDRCOMMS_APPLICATIONPROCESSOR_H
