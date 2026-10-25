//
// Created by Chris Davey on 25/10/2026.
//

#ifndef SDRCOMMS_MODELFILELOADER_H
#define SDRCOMMS_MODELFILELOADER_H
#include "IQBlock.h"
#include "ModelFileSpec.h"
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
        ModelFileLoader(std::string basePath, const ModelFileSpec& modelFileSpec);

        ~ModelFileLoader() = default;

        /**
         * initialise the onnx runtime environment and session
         * @return
         */
        bool load_model();

        /**
         * given the input data execute the prediction
         * and return the result.
         * @param input
         * @return
         */
        std::vector<float> predict(const IQBlock& input);


    private:
        std::string modelBasePath;
        std::string modelFilePath;
        ModelFileSpec modelFileSpec;
        std::shared_ptr<Ort::Env> env;
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
        bool check_model_input_attachments(const IQBlock & block) const;
    };
} // sdrcomms

#endif //SDRCOMMS_MODELFILELOADER_H
