//
// Created by Chris Davey on 25/10/2026.
//

#include "include/ModelFileLoader.h"

#include <iostream>
#include <map>
#include <ranges>
#include <algorithm>
#include <set>

namespace sdrcomms {
    /**
         * instantiate with the base path where model files are found
         * and the file spec that is used to define the model file.
         * @param basePath
         * @param modelFileSpec
         */
    ModelFileLoader::ModelFileLoader(std::string basePath, const ModelFileSpec &modelFileSpec) {
        this->modelFileSpec = modelFileSpec;
        this->modelBasePath = basePath;
        if (basePath.ends_with("/")) {
            this->modelFilePath = basePath + modelFileSpec.onnx_file;
        } else {
            this->modelFilePath = basePath + "/" + modelFileSpec.onnx_file;
        }
    }


    /**
     * initialise the onnx runtime environment and session
     * @return
     */
    bool ModelFileLoader::load_model() {
        try {
            env = std::make_shared<Ort::Env>(ORT_LOGGING_LEVEL_WARNING, "sdrcomms_onnx_host");
            Ort::SessionOptions session_options;
            session_options.SetIntraOpNumThreads(1);
            session_options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_EXTENDED);
            session = std::make_unique<Ort::Session>(env, modelFilePath.c_str(), session_options);
        } catch (std::exception e) {
            std::cerr << "Error loading model: " << e.what() << std::endl;
            return false;
        }
        try {
            // get the input shape details.
            Ort::AllocatorWithDefaultOptions allocator;
            auto input_info = session->GetInputTypeInfo(0);
            auto input_tensor_info = input_info.GetTensorTypeAndShapeInfo();
            inputNodeDims = input_tensor_info.GetShape();
            auto allocated_input_name = session->GetInputNameAllocated(0, allocator);
            inputNodeName = std::string(allocated_input_name.get());
            auto allocated_output_name = session->GetOutputNameAllocated(0, allocator);
            outputNodeName = std::string(allocated_output_name.get());

            auto output_info = session->GetOutputTypeInfo(0);
            auto output_tensor_info = output_info.GetTensorTypeAndShapeInfo();
            outputNodeDims = output_tensor_info.GetShape();

            std::cout << "[Model Loader] Loaded model " << modelFilePath << std::endl;
            std::cout << "Model Name: " << modelFileSpec.name << " Version: " << modelFileSpec.version << std::endl;
            std::cout << "Input Node Name: " << inputNodeName << " Output Node Name: " << outputNodeName << std::endl;
            std::cout << "Input Shape: " << inputNodeDims << " Output Shape: " << outputNodeDims << std::endl;
            return true;
        } catch (std::exception &e) {
            std::cerr << "Could not obtain model input and output dimensions: " << e.what() << std::endl;
            return false;
        }
    }

    /**
     * given the input data execute the prediction
     * and return the result.
     * @param input
     * @return
     */
    std::vector<float> ModelFileLoader::predict(const IQBlock &input) {
        if (!check_model_input_attachments(input)) {
            return {};
        }
        // now we need to extract all input blocks
    }

    /**
         * check that the block contains all input attachments.
         * @param block
         * @return
         */
    bool ModelFileLoader::check_model_input_attachments(const IQBlock &block) const {
        std::map<std::string, int> expected = modelFileSpec.input_transforms;
        auto expected_keys = std::views::keys(expected) | std::ranges::to<std::set<std::string>>();
        auto actual_keys = std::views::keys(block.attachments) | std::ranges::to<std::set<std::string>>();
        bool matches = std::ranges::includes(actual_keys, expected_keys);
        return matches;
    }
} // sdrcomms
