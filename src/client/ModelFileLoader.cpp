//
// Created by Chris Davey on 25/10/2026.
//

#include "include/ModelFileLoader.h"

#include <iostream>
#include <map>
#include <ranges>
#include <algorithm>
#include <glob.h>
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
            // only initialise the environment once.
            env = global_env();
            Ort::SessionOptions session_options;
            session_options.SetIntraOpNumThreads(1);
            session_options.SetGraphOptimizationLevel(ORT_ENABLE_EXTENDED);

            backend = detect_backend();
            if (backend == InferenceBackend::CUDA) {
                OrtCUDAProviderOptions cuda_options;
                // device ID defaulting to 0, could instead make this a runtime parameter.
                cuda_options.device_id = 0;
                try {
                    session_options.AppendExecutionProvider_CUDA(cuda_options);
                    std::cout << "Successfully bound CUDA accelerator.\n";
                } catch (std::exception &e) {
                    std::cerr << "CUDA failed to bind, falling back to default CPU. Details: " << e.what() << "\n";
                    backend = InferenceBackend::CPU;
                }
            } else if (backend == InferenceBackend::ROCM) {
                OrtROCMProviderOptions rocm_options;
                rocm_options.device_id = 0;
                try {
                    session_options.AppendExecutionProvider_ROCM(rocm_options);
                    std::cout << "Successfully bound ROCM accelerator.\n";
                } catch (std::exception &e) {
                    std::cerr << "ROCM failed to bind, falling back to default CPU. Details: " << e.what() << "\n";
                    backend = InferenceBackend::CPU;
                }
            } else if (backend == InferenceBackend::CoreML) {
                std::unordered_map<std::string, std::string> coreml_options;
                // Choose what hardware chips to leverage: "CPU_ONLY", "CPU_AND_GPU", or "ALL" (ANE + GPU + CPU)
                coreml_options["MLComputeUnits"] = "ALL";
                // Use the modern compilation format for Apple Silicon
                coreml_options["ModelFormat"] = "MLProgram";
                try {
                    // 2. Append using the generic string factory (Available natively on macOS/iOS builds)
                    session_options.AppendExecutionProvider("CoreML", coreml_options);
                    std::cout << "Successfully bound CoreML (Metal/Apple Silicon) accelerator.\n";
                } catch (const Ort::Exception &e) {
                    std::cerr << "CoreML failed to bind, falling back to default CPU. Details: " << e.what() << "\n";
                    backend = InferenceBackend::CPU;
                }
            } else if (backend == InferenceBackend::WebGPU) {
                std::unordered_map<std::string, std::string> webgpu_options = {
                    {"device_id", "0"}, // Explicitly tells ONNX to use the primary GPU (Default)
                    {"preferredLayout", "NHWC"},
                    {"enableInt64", "1"} // needed if ONNX model uses int64 tensors
                };

                try {
                    // 2. Simply pass the backend string token.
                    // ONNX Runtime auto-discovers and binds the physical GPU natively!
                    session_options.AppendExecutionProvider("WebGPU", webgpu_options);
                    std::cout << "Successfully bound WebGPU using the native string API.\n";
                } catch (const Ort::Exception &e) {
                    std::cerr << "Failed to initialize WebGPU: " << e.what() << ". Falling back to CPU.\n";
                    backend = InferenceBackend::CPU;
                }
            }

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

            return true;
        } catch (std::exception &e) {
            std::cerr << "Could not obtain model input and output dimensions: " << e.what() << std::endl;
            return false;
        }
    }

    /**
         * unload the session and environment
         * @return
         */
    bool ModelFileLoader::unload() {
        if (session != nullptr) {
            session.reset();
            session = nullptr;
            return true;
        }
        // nothing to unload
        return true;
    }

    /**
     * search for providers available to the system, pick GPU first and fallback to CPU if not available
     * @return
     */
    ModelFileLoader::InferenceBackend ModelFileLoader::detect_backend() const {
        std::vector<std::string> available = Ort::GetAvailableProviders();
        if (std::find(available.begin(), available.end(), "CUDAExecutionProvider") != available.end()) {
            return InferenceBackend::CUDA;
        }
        if (std::find(available.begin(), available.end(), "ROCMExecutionProvider") != available.end()) {
            return InferenceBackend::ROCM;
        }
        if (std::find(available.begin(), available.end(), "CoreMLExecutionProvider") != available.end()) {
            return InferenceBackend::CoreML;
        }
        if (std::find(available.begin(), available.end(), "WebGPUExecutionProvider") != available.end()) {
            return InferenceBackend::WebGPU;
        }
        // CPU provider is always available.
        if (std::find(available.begin(), available.end(), "CPUExecutionProvider") != available.end()) {
            return InferenceBackend::CPU;
        }
        return InferenceBackend::Auto;
    }

    /**
     * given the input data execute the prediction
     * and return the result.
     * @param input
     * @return
     */
    std::vector<float> ModelFileLoader::predict(const IQBlock &input) {
        if (session == nullptr) {
            return {};
        }
        if (!check_model_input_attachments(input)) {
            return {};
        }
        // first determine if the keys added into the input block attachments match the data input required for the model.
        if (!check_model_input_attachments(block)) {
            // inputs are not supplied
#ifdef DEBUG_VERBOSE
            std::cerr << "Model input attachments do not match the required input for the model." << std::endl;
#endif
            return {};
        }
        // now we need to extract all input blocks
        auto input_blocks = get_input_block_attachments(block.attachments);
        auto input_data = get_input_data(input_blocks);
        // now we want to use this input data as the input to the model.
        return predict(input_data);
    }

    /**
          * predict with 2d input vectors
          * @param input
          * @return
          */
    std::vector<float> ModelFileLoader::predict(const std::vector<std::vector<float> > &input_vector) {
        // Basic verification check: Handle flat batch size logic assumptions dynamically
        if (inputNodeDims[0] == -1 || inputNodeDims[0] == 0) {
            inputNodeDims[0] = 1;
        }
        std::vector<float> flat_input_vector = flatten_2d_tensor(input_vector);
        std::vector<int64_t> prediction_shape = {1, inputNodeDims[1], inputNodeDims[2]};
        // Force shape alignment security checks
        size_t expected_elements = 1;
        for (auto dim: prediction_shape) expected_elements *= dim;

        if (flat_input_vector.size() != expected_elements) {
            std::cerr << "[ONNX Host] Error: Vector elements count mismatch.\n";
            return {};
        }

        try {
            if (backend == InferenceBackend::CUDA) {
                Ort::MemoryInfo memory_info(
                    "Cuda",
                    OrtAllocatorType::OrtDeviceAllocator,
                    0,
                    OrtMemType::OrtMemTypeDefault);
                return predict(memory_info, prediction_shape, flat_input_vector);
            }

            if (backend == InferenceBackend::ROCM) {
                Ort::MemoryInfo memory_info(
                    "Rocm",
                    OrtAllocatorType::OrtDeviceAllocator,
                    0,
                    OrtMemType::OrtMemTypeDefault);
                return predict(memory_info, prediction_shape, flat_input_vector);
            }

            if (backend == InferenceBackend::WebGPU) {
                Ort::MemoryInfo memory_info(
                    "WebGpu",
                    OrtAllocatorType::OrtDeviceAllocator,
                    0,
                    OrtMemType::OrtMemTypeDefault);
                return predict(memory_info, prediction_shape, flat_input_vector);
            }

            Ort::MemoryInfo memory_info = Ort::MemoryInfo::CreateCpu(OrtAllocatorType::OrtArenaAllocator,
                                                                     OrtMemType::OrtMemTypeDefault);
            return predict(memory_info, prediction_shape, flat_input_vector);
        } catch (const Ort::Exception &e) {
            std::cerr << "[ONNX Host] Runtime Inference Error: " << e.what() << "\n";
            return {};
        }
    }

    /**
        * predict with memory info, prediction dim and flat vector
        * @param input
        * @return
        */
    std::vector<float> ModelFileLoader::predict(const Ort::MemoryInfo &memory_info,
                                                const std::vector<int64_t> &prediction_shape,
                                                const std::vector<float> &flat_input_vector) {
        // 4. Wrap your input vector memory span straight into an ONNX Runtime tensor view without reallocation
        Ort::Value input_tensor = Ort::Value::CreateTensor<float>(
            memory_info,
            const_cast<float *>(flat_input_vector.data()),
            flat_input_vector.size(),
            prediction_shape.data(),
            prediction_shape.size()
        );

        const char *input_names[] = {inputNodeName.c_str()};
        const char *output_names[] = {outputNodeName.c_str()};

        // 5. Run execution inference pipeline blocks
        auto output_tensors = session->Run(
            Ort::RunOptions{nullptr},
            input_names, &input_tensor, 1,
            output_names, 1
        );

        // Extract internal logits float values out of the output array wrapper
        float *raw_output_data = output_tensors[0].GetTensorMutableData<float>();
        auto output_info = output_tensors[0].GetTensorTypeAndShapeInfo();
        size_t output_size = output_info.GetElementCount();

        if (modelFileSpec.logits) {
            std::vector<float> raw_logits(raw_output_data, raw_output_data + output_size);

            // 6. Return values pushed directly through the Softmax mathematical translation layer
            return compute_softmax(raw_logits);
        } else {
            std::vector<float> softmax_output(raw_output_data, raw_output_data + output_size);
            return softmax_output;
        }
    }

    /**
         * flatten nested vectors into a single flat vector for use with ONNX Runtime.
         * @param nested_matrix
         * @return
         */
    std::vector<float> ModelFileLoader::flatten_2d_tensor(const std::vector<std::vector<float> > &nested_matrix) const {
        std::vector<float> flat_vector;

        // 1. Calculate total elements across all sub-vectors up-front
        size_t total_elements = 0;
        for (const auto &row: nested_matrix) {
            total_elements += row.size();
        }

        // 2. Allocate the exact block of memory at once (Zero reallocations!)
        flat_vector.reserve(total_elements);

        // 3. Batch-insert each inner vector's memory block sequentially
        for (const auto &row: nested_matrix) {
            flat_vector.insert(flat_vector.end(), row.begin(), row.end());
        }

        return flat_vector;
    }

    /**
        * Convert logits to softmax probabilities.
        * @param raw_logits
        * @return
        */
    std::vector<float> ModelFileLoader::compute_softmax(const std::vector<float> &raw_logits) const {
        std::vector<float> softmax_outputs(raw_logits.size());

        // Stabilize math against floating-point overflows by subtracting the maximum logit element value
        float max_logit = *std::max_element(raw_logits.begin(), raw_logits.end());

        float denominator_sum = 0.0f;
        for (size_t i = 0; i < raw_logits.size(); ++i) {
            softmax_outputs[i] = std::exp(raw_logits[i] - max_logit);
            denominator_sum += softmax_outputs[i];
        }

        // Convert components into strict probability scores
        for (size_t i = 0; i < softmax_outputs.size(); ++i) {
            softmax_outputs[i] /= denominator_sum;
        }

        return softmax_outputs;
    }

    /**
         * check that the block contains all input attachments.
         * @param block
         * @return
         */
    bool ModelFileLoader::check_model_input_attachments(const IQBlock &block) const {
        auto expected_keys = modelFileSpec.input_transforms | std::ranges::to<std::set<std::string> >();
        auto actual_keys = std::views::keys(block.attachments) | std::ranges::to<std::set<std::string> >();
        bool matches = std::ranges::includes(actual_keys, expected_keys);
        return matches;
    }

    /**
     * get the set of input blocks required for the model.
     * @param block
     * @return
     */
    std::map<std::string, std::shared_ptr<IQBlock> >
    ModelFileLoader::get_input_block_attachments(const IQBlock &block) {
        std::map<std::string, std::shared_ptr<IQBlock> > input_blocks{};
        auto expected_keys = modelFileSpec.input_transforms | std::ranges::to<std::set<std::string> >();
        for (auto key: expected_keys) {
            auto inner = block.attachments.at(key);
            input_blocks[key] = inner;
        }
        return input_blocks;
    }

    /**
         * extract the series of  values from the preprocessed input blocks expected by the model.
         * @param input
         * @return
         */
    std::vector<std::vector<float> > ModelFileLoader::get_input_data(
        const std::map<std::string, std::shared_ptr<IQBlock> > &input) const {
        std::vector<std::vector<float> > input_data{};
        // for each input block transform we convert the each complex block sample vector
        // into two vectors of real and imaginary numbers
        // then add those sequentially into the outer vector.
        // this is expected to create a 2d matrix
        for (auto key: modelFileSpec.input_transforms) {
            std::shared_ptr<IQBlock> inner = input.at(key);
            size_t n = inner->samples.size();
            std::vector<float> re(n);
            std::vector<float> im(n);
            std::ranges::generate(re, [inner, i=0]() mutable {
                float value = inner->samples[i].real();
                i++;
                return value;
            });
            std::ranges::generate(im, [inner, i=0]() mutable {
                float value = inner->samples[i].imag();
                i++;
                return value;
            });
            input_data.push_back(re);
            input_data.push_back(im);
        }
        return input_data;
    }
} // sdrcomms
