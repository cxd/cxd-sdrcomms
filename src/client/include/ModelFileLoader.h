//
// Created by Chris Davey on 25/10/2026.
//

#ifndef SDRCOMMS_MODELFILELOADER_H
#define SDRCOMMS_MODELFILELOADER_H
#include "IQBlock.h"
#include "ModelFileSpec.h"
#include "ProcessorOps.h"
#include "onnxruntime/onnxruntime_cxx_api.h"

namespace sdrcomms {
    class ModelFileLoader {
    public:
        /**
         * instantiate with the base path where model files are found
         * and the file spec that is used to define the model file.
         * @param basePath
         * @param modelFileSpec
         */
        ModelFileLoader(std::string basePath, const ModelFileSpec& modelFileSpec, std::vector<ProcessorOps> &processorOps );

        ~ModelFileLoader() = default;

        /**
         * initialise the onnx runtime environment and session
         * @return
         */
        bool load_model();

        /**
         * unload the session and environment
         * @return
         */
        bool unload();

        /**
         * given the input data execute the prediction
         * and return the result.
         * @param input
         * @return
         */
        std::vector<float> predict(const IQBlock& input, std::map<std::string, std::shared_ptr<sdrcomms::IQBlock>> &attachments);

        /**
         * predict with 2d input vectors
         * @param input
         * @return
         */
        std::vector<float> predict(const std::vector<std::vector<float>> &input);

        // static initialiser
        static Ort::Env& global_env() {
            static Ort::Env global_env(ORT_LOGGING_LEVEL_WARNING, "sdrcomms_onnx_host");
            return global_env;
        }


    private:
        std::string modelBasePath;
        std::string modelFilePath;
        ModelFileSpec modelFileSpec;
        std::vector<ProcessorOps> processorOps {};
        Ort::Env& env = global_env();
        std::shared_ptr<Ort::Session> session;
        std::vector<int64_t> inputNodeDims;
        std::vector<int64_t> outputNodeDims;
        std::string inputNodeName;
        std::string outputNodeName;

        /**
         * check that the block contains all input attachments.
         * @param block
         * @return
         */
        bool check_model_input_attachments(std::map<std::string, std::shared_ptr<sdrcomms::IQBlock>> &attachments) const;

        std::map<std::string, std::shared_ptr<IQBlock>> get_input_block_attachments(std::map<std::string, std::shared_ptr<sdrcomms::IQBlock>> &attachments);

        /**
         * extract the series of  values from the preprocessed input blocks expected by the model.
         * @param input
         * @return
         */
        std::vector<std::vector<float>> get_input_data(const std::map<std::string, std::shared_ptr<IQBlock>>& input) const;

        /**
         * flatten nested vectors into a single flat vector for use with ONNX Runtime.
         * @param nested_matrix
         * @return
         */
        std::vector<float> flatten_2d_tensor(const std::vector<std::vector<float>>& nested_matrix) const;

        /**
         * Convert logits to softmax probabilities.
         * @param raw_logits
         * @return
         */
        std::vector<float> compute_softmax(const std::vector<float>& raw_logits) const;

        /**
         * predict with memory info, prediction dim and flat vector
         * @param input
         * @return
         */
        std::vector<float> predict(const Ort::MemoryInfo &memory_info, const std::vector<int64_t> &prediction_shape, const std::vector<float> &flat_input_vector);

        enum class InferenceBackend {
            Auto = 0,
            CUDA,
            ROCM,
            CoreML,
            WebGPU,
            CPU
        };

        InferenceBackend backend = InferenceBackend::Auto;

        InferenceBackend detect_backend() const;

    };
} // sdrcomms

#endif //SDRCOMMS_MODELFILELOADER_H
