//
// Created by Chris Davey on 30/10/2026.
//

#include "include/ClassificationModelProcessor.h"

#include <iostream>

namespace sdrcomms {
    ClassificationModelProcessor::ClassificationModelProcessor(std::string attach_key,
                                                               std::string basePath,
                                                               const ModelFileSpec &modelSpec) {
        this->attach_key = attach_key;
        this->basePath = basePath;
        this->modelSpec = modelSpec;
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
        modelFileLoader = std::make_unique<ModelFileLoader>(basePath, modelSpec);
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
    void ClassificationModelProcessor::process(IQBlock &block) {
        if (!isInit) {
            return;
        }
        // TODO: we should add a signal detection mechanism to determine if the classifier should be called.
        // execute classification.
        this->collectAttachments(block);
        std::vector<float> probabilities = modelFileLoader->predict(block);
        // TODO: we need to pick the maximum index and read the class label for that index from the class labels file.
        // that could be done as part of prediction method inside modelFileLoader.

        // TODO: after prediction we need to send a control message indicating classification outputs.
        // we need a channel to communicate this on, ie: a concurrent queue for control messages.
    }
}
