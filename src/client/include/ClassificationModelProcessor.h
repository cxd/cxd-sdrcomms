//
// Created by Chris Davey on 30/10/2026.
//

#ifndef SDRCOMMS_CLASSIFICATIONMODELPROCESSOR_H
#define SDRCOMMS_CLASSIFICATIONMODELPROCESSOR_H
#include "ModelFileLoader.h"
#include "ModelFileSpec.h"
#include "ProcessorOps.h"
#include "SampleProcessor.h"

namespace sdrcomms {
    class ClassificationModelProcessor : public SampleProcessor {
    public:
        ClassificationModelProcessor(std::string attach_key,  std::string basePath, const ModelFileSpec &modelSpec);

        /**
         * initialise the processor
         * @return
         */
        bool init();

        /**
         * process the incoming block and update the inner processor.
         * @param block
         */
        void process(IQBlock &block) override;

    private:
        bool isInit = false;
        std::string basePath;
        std::string modelPath;
        ModelFileSpec modelSpec{};
        std::unique_ptr<ModelFileLoader> modelFileLoader;

        /**
         * initialise the model.
         * @return
         */
        bool load_model();

    };
} // sdrcomms

#endif //SDRCOMMS_CLASSIFICATIONMODELPROCESSOR_H
