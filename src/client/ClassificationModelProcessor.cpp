//
// Created by Chris Davey on 30/10/2026.
//

#include "include/ClassificationModelProcessor.h"

#include <iostream>

namespace sdrcomms {

    ClassificationModelProcessor::ClassificationModelProcessor(std::string attach_key,
        std::string basePath,
        const ModelFileSpec &modelSpec,
        const ClassMap &classMap,
        const std::vector<ProcessorOps> &processorOps,
        std::shared_ptr<MessageQueue<sdrcomms::ControlMessage>> eventQueue) {
        this->attach_key = attach_key;
        this->basePath = basePath;
        this->modelSpec = modelSpec;
        this->classMap = classMap;
        this->processorOps = processorOps;
        // the event queue can be used to post classification events on a separate thread.
        this->eventQueue = eventQueue;
    }

    /**
        * initialise the processor
        * @return
        */
    bool ClassificationModelProcessor::init() {
        isInit = load_model();
        if (!isInit) {
            return isInit;
        }
        isRunning.store(true, std::memory_order_relaxed);
        return isInit;
    }

    void ClassificationModelProcessor::teardown() {
        if (isRunning.load(std::memory_order_relaxed)) {
            SampleProcessor::teardown();
            isRunning.store(false, std::memory_order_relaxed);
            if (classifyThread.joinable()) {
                classifyThread.join();
            }
        }
    }

    /**
         * initialise the model.
         * @return
         */
    bool ClassificationModelProcessor::load_model() {
        if (basePath.ends_with('/')) {
            basePath.pop_back();
        }
        modelPath = basePath + "/" + modelSpec.onnx_file;
        modelFileLoader = std::make_unique<ModelFileLoader>(basePath, modelSpec, processorOps);
        if (!modelFileLoader->load_model()) {
            std::cerr << "Failed to load model file: " << modelPath << "/" << modelSpec.onnx_file << std::endl;
            return false;
        }
        return true;
    }


    /**
         * process the incoming block and update the inner processor.
         * @param block
         */
    void ClassificationModelProcessor::process(IQBlock &block, std::map<std::string, std::shared_ptr<IQBlock>> &attachments) {
        if (!isInit) {
            return;
        }
        // TODO: we should add a signal detection mechanism to determine if the classifier should be called execute classification.
        // signal detection can be run as a processor block before this process block.
        block.attachments = attachments;
        this->classifyQueue->push(block);
    }


    /**
         * classification worker thread
         */
    void ClassificationModelProcessor::classify_worker() {
        while (isRunning.load(std::memory_order_relaxed)) {
            IQBlock block {};
            if (classifyQueue->pop(block)) {
                auto attachments = block.attachments;
                if (attachments.size() > 0) {
                    // perform inference
                    std::vector<float> probabilities = modelFileLoader->predict(block, attachments);
                    // TODO: get maximum propbabilities

                    // TODO: post classification message into the event queue

                }

            }
        }
    }
}
