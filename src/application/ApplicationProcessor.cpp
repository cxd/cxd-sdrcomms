//
// Created by Chris Davey on 29/9/2026.
//

#include "ApplicationProcessor.h"

#include <iostream>
#include <ranges>

#include "ClassificationModelProcessor.h"
#include "constants.h"
#include "DownsampleProcessor.h"
#include "RootMeanSquareProcessor.h"

ApplicationProcessor::ApplicationProcessor(std::string udpServerAddress, int udpPort,
                                           double source_sample_rate_hz,
                                           float rms_db_threshold,
                                           size_t required_samples,
                                           std::string modelFile,
                                           std::string classMapFilename,
                                           std::string basePath,
                                           std::shared_ptr<sdrcomms::MessageQueue<sdrcomms::ControlMessage> >
                                           eventQueue) {
    this->udpServerAddress = udpServerAddress;
    this->udpPort = udpPort;
    this->source_sample_rate_hz = source_sample_rate_hz;
    this->rms_db_threshold = rms_db_threshold;
    this->required_samples = required_samples;
    this->modelSpecFile = modelFile;
    this->classMapFilename = classMapFilename;
    this->basePath = basePath;
    this->modelPath = "";
    this->eventQueue = eventQueue;
}

/**
     * initialise the application processor.
     * @return
     */
bool ApplicationProcessor::init() {
    // load the json model file and the model file.
    modelSpec = sdrcomms::ModelFileSpec::from_file(modelSpecFile);
    classMap = sdrcomms::ClassMap::from_file(classMapFilename);

    // load the process operations.
    processorOps = get_operations();
    // create the internal classification model processor
    std::shared_ptr<sdrcomms::SampleProcessor> last_processor = make_model_processor();
    // processors defined by the model spec file.
    processingChain = build_processing_chain(last_processor);
    // udp stream processing chain
    this->sampleStreamerProcessor = std::make_unique<sdrcomms::SampleStreamerProcessor>(
        udpServerAddress, udpPort, required_samples);
    return true;
}

/**
* tear down the processor chain
*/
void ApplicationProcessor::teardown() {
    this->processingChain->teardown();
}

/**
 * get the processor operations from the model spec.
 * @return
 */
std::vector<sdrcomms::ProcessorOps> ApplicationProcessor::get_operations() {
    std::vector<sdrcomms::ProcessorOps> result{};
    for (const auto &transform: modelSpec.input_transforms) {
        sdrcomms::ProcessorOps op = sdrcomms::operation_from_string(transform);
        if (op == sdrcomms::ProcessorOps::NONE) {
            continue;
        }
        result.push_back(op);
    }
    return result;
}

/**
     * initialise the model processor.
     * @return
     */
std::shared_ptr<sdrcomms::SampleProcessor> ApplicationProcessor::make_model_processor() {
    std::string op = sdrcomms::operation_to_string(sdrcomms::ProcessorOps::CLASSIFY);
    //  attach an RMS block into the processing chain before the downsampler.
    std::string downsampleOp = sdrcomms::operation_to_string(sdrcomms::ProcessorOps::DOWNSAMPLE);
    auto rms_processor = std::make_shared<sdrcomms::RootMeanSquareProcessor>(rms_key, downsampleOp, nullptr);
    modelProcessor = std::make_shared<sdrcomms::ClassificationModelProcessor>(op,
                                                                              basePath,
                                                                              modelSpec,
                                                                              classMap,
                                                                              rms_key,
                                                                              rms_db_threshold,
                                                                              rms_processor,
                                                                              processorOps,
                                                                              eventQueue);
    if (!modelProcessor->init()) {
        return nullptr;
    }
    return modelProcessor;
}


/**
 * build the processing chain.
 * We will have to work backwards from the last item in the processing ops back to the first.
 */
std::shared_ptr<sdrcomms::SampleProcessor> ApplicationProcessor::build_processing_chain(
    std::shared_ptr<SampleProcessor> lastProcessor) {
    for (auto op: processorOps | std::ranges::views::reverse) {
        if (op == sdrcomms::ProcessorOps::DOWNSAMPLE) {
            const std::shared_ptr<SampleProcessor> next = std::make_shared<sdrcomms::DownsampleProcessor>(
                sdrcomms::operation_to_string(op),
                modelSpec.input_length, // the target sample size needed for the classifier.
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
void ApplicationProcessor::process(sdrcomms::IQBlock &block,
                                   std::map<std::string, std::shared_ptr<sdrcomms::IQBlock> > &attachments) {
    auto now = std::chrono::system_clock::now();
    auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(now - this->last_packet).count();
    if (delta >= ONE_SECOND) {
        last_packet = now;
        std::cout << "Blocks per second: " << this->blocks_per_second << std::endl;
#ifdef DEBUG_VERBOSE
        std::cout << "Blocks per second: " << this->blocks_per_second << std::endl;
#endif
        this->blocks_per_second = 0;
    }
    this->blocks_per_second++;
    // processing chain
    this->processingChain->process(block, attachments);
    // streaming iq samples
    this->sampleStreamerProcessor->process(block, attachments);
}
