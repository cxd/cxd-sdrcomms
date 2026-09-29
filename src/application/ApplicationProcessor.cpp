//
// Created by Chris Davey on 29/9/2026.
//

#include "ApplicationProcessor.h"

ApplicationProcessor::ApplicationProcessor(int udpPort) {
    this->udpPort = udpPort;
    this->mockProcessor = std::make_unique<MockProcessor>();
    this->sampleStreamerProcessor = std::make_unique<sdrcomms::SampleStreamerProcessor>(udpPort);
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
void ApplicationProcessor::process(const sdrcomms::IQBlock &block) {
    this->mockProcessor->process(block);
    this->sampleStreamerProcessor->process(block);
}
