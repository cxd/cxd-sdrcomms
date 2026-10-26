//
// Created by Chris Davey on 29/9/2026.
//

#include "ApplicationProcessor.h"

#include <iostream>

#include "constants.h"

ApplicationProcessor::ApplicationProcessor(std::string udpServerAddress, int udpPort, std::string modelFile, std::string modelPath) {
    this->udpServerAddress = udpServerAddress;
    this->udpPort = udpPort;
    this->mockProcessor = std::make_unique<MockProcessor>();
    this->sampleStreamerProcessor = std::make_unique<sdrcomms::SampleStreamerProcessor>(udpServerAddress, udpPort);
    this->fftProcessor = std::make_unique<sdrcomms::FFTSampleProcessor>("fft", sdrcomms::REQUIRED_SAMPLES, nullptr);
    this->modelSpecFile = modelFile;
    this->modelPath = modelPath;
}

void ApplicationProcessor::startServer() {
    this->sampleStreamerProcessor->startServer();
}

void ApplicationProcessor::stopServer() {
    this->sampleStreamerProcessor->stopServer();
}


/**
     * implement the process method
     * @param block
     */
void ApplicationProcessor::process(sdrcomms::IQBlock &block) {
    auto now = std::chrono::system_clock::now();
    auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->last_packet).count();
    if (delta >= ONE_SECOND) {
        last_packet = now;
#ifdef DEBUG_VERBOSE
        std::cout << "Blocks per second: " << this->blocks_per_second << std::endl;
#endif
        this->blocks_per_second = 0;
    }
    this->blocks_per_second++;
    this->mockProcessor->process(block);
    // try processing the fft.
    this->fftProcessor->process(block);
    this->sampleStreamerProcessor->process(block);
}
