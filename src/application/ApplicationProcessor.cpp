//
// Created by Chris Davey on 29/9/2026.
//

#include "ApplicationProcessor.h"

#include <iostream>
#include <ranges>

#include "constants.h"
#include "DownsampleProcessor.h"

ApplicationProcessor::ApplicationProcessor(std::string udpServerAddress, int udpPort, double source_sample_rate_hz,
                                           std::string modelFile,
                                           std::string basePath) {
    this->udpServerAddress = udpServerAddress;
    this->udpPort = udpPort;
    this->source_sample_rate_hz = source_sample_rate_hz;
    this->modelSpecFile = modelFile;
    this->basePath = basePath;
    this->modelPath = "";
}

/**
     * initialise the application processor.
     * @return
     */
bool ApplicationProcessor::init() {
    if (!load_model()) {
        return false;
    }
    // load the process operations.
    processorOps = get_operations();
    // processors defined by the model spec file.
    processingChain = build_processing_chain();
    // udp stream processing chain
    this->sampleStreamerProcessor = std::make_unique<sdrcomms::SampleStreamerProcessor>(udpServerAddress, udpPort);

    return false;
}

/**
    * initialise the model.
    * @return
    */
bool ApplicationProcessor::load_model() {
    // load the json model file and the model file.
    modelSpec = sdrcomms::ModelFileSpec::from_file(modelSpecFile);
    if (basePath.ends_with('/')) {
        basePath.pop_back();
    }
    modelPath = basePath + "/" + modelSpec.onnx_file;
    modelFileLoader = std::make_unique<sdrcomms::ModelFileLoader>(modelPath, modelSpec);
    if (!modelFileLoader->load_model()) {
        std::cerr << "Failed to load model file: " << modelPath << "/" << modelSpec.onnx_file << std::endl;
        return false;
    }
    return true;
}

/**
 * get the processor operations from the model spec.
 * @return
 */
std::vector<sdrcomms::ProcessorOps> ApplicationProcessor::get_operations() {
    std::vector<sdrcomms::ProcessorOps> result{};
    for (const auto& transform: modelSpec.input_transforms) {
        sdrcomms::ProcessorOps op = sdrcomms::operation_from_string(transform);
        if (op == sdrcomms::ProcessorOps::NONE) {
            continue;
        }
        result.push_back(op);
    }
    return result;
}

/**
 * build the processing chain.
 * We will have to work backwards from the last item in the processing ops back to the first.
 */
std::shared_ptr<sdrcomms::SampleProcessor> ApplicationProcessor::build_processing_chain() {
    std::shared_ptr<SampleProcessor> lastProcessor = nullptr;
    for (auto op: processorOps | std::ranges::views::reverse) {
        if (op == sdrcomms::ProcessorOps::DOWNSAMPLE) {
            const std::shared_ptr<SampleProcessor> next = std::make_shared<sdrcomms::DownsampleProcessor>(
                sdrcomms::operation_to_string(op),
                source_sample_rate_hz,
                modelSpec.sample_rate_hz,
                lastProcessor);
            lastProcessor = next;
            continue;
        }
        if (op == sdrcomms::ProcessorOps::FFT) {
            // we need the target size of the FFT based on our model spec.
            const std::shared_ptr<SampleProcessor> next = std::make_shared<sdrcomms::FFTSampleProcessor>(
                sdrcomms::operation_to_string(op),
                modelSpec.input_length,
                lastProcessor);
            lastProcessor = next;
            continue;
        }
    }
    return lastProcessor;
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



    // processing chain
    this->processingChain->process(block);
    // streaming iq samples
    this->sampleStreamerProcessor->process(block);

    // TODO: we should add a signal detection mechanism to determine if the classifier should be called.
    // execute classification.
    std::vector<float> probabilities = modelFileLoader->predict(block);
    // TODO: we need to pick the maximum index and read the class label for that index from the class labels file.
    // that could be done as part of prediction method inside modelFileLoader.

}
