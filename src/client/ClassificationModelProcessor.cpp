//
// Created by Chris Davey on 30/10/2026.
//

#include "include/ClassificationModelProcessor.h"

#include <iostream>

namespace sdrcomms {

    ClassificationModelProcessor::ClassificationModelProcessor(std::string attach_key,
        std::string basePath,
        const ModelFileSpec &modelSpec,
        const std::vector<ProcessorOps> &processorOps,
        std::shared_ptr<MessageQueue<sdrcomms::ControlMessage>> eventQueue) {
        this->attach_key = attach_key;
        this->basePath = basePath;
        this->modelSpec = modelSpec;
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
        return isInit;
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
        // TODO: we should add a signal detection mechanism to determine if the classifier should be called  execute classification.
        // TODO: we might need to also run the prediction process in a separate thread this is because the forward pass on ONNX seems to be somewhat slow.
        // this will mean we need a queue message type for the inputs to the prediction method.
        //
        std::vector<float> probabilities = modelFileLoader->predict(block, attachments);
        // TODO: we need to pick the maximum index and read the class label for that index from the class labels file.
        // that could be done as part of prediction method inside modelFileLoader.

        // TODO: after prediction we need to send a control message indicating classification outputs.
        // we need a channel to communicate this on, ie: a concurrent queue for control messages.

        // forwardAttachments(block, attach_key, innerBlock);
    }
}
