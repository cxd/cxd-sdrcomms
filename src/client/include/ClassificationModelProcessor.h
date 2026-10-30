//
// Created by Chris Davey on 30/10/2026.
//

#ifndef SDRCOMMS_CLASSIFICATIONMODELPROCESSOR_H
#define SDRCOMMS_CLASSIFICATIONMODELPROCESSOR_H
#include "ControlMessage.h"
#include "MessageQueue.h"
#include "ModelFileLoader.h"
#include "ModelFileSpec.h"
#include "ProcessorOps.h"
#include "SampleProcessor.h"

namespace sdrcomms {
    class ClassificationModelProcessor : public SampleProcessor {
    public:
        ClassificationModelProcessor(std::string attach_key,  std::string basePath, const ModelFileSpec &modelSpec, const std::vector<ProcessorOps> &processorOps,
            std::shared_ptr<MessageQueue<sdrcomms::ControlMessage>> eventQueue);

        /**
         * initialise the processor
         * @return
         */
        bool init();

        /**
         * process the incoming block and update the inner processor.
         * @param block
         */
        void process(IQBlock &block, std::map<std::string, std::shared_ptr<IQBlock>> &attachments) override;

    private:
        bool isInit = false;
        std::string basePath;
        std::string modelPath;
        ModelFileSpec modelSpec{};
        std::vector<ProcessorOps> processorOps;
        std::unique_ptr<ModelFileLoader> modelFileLoader;
        std::shared_ptr<sdrcomms::MessageQueue<sdrcomms::ControlMessage>> eventQueue;
        /**
         * initialise the model.
         * @return
         */
        bool load_model();

    };
} // sdrcomms

#endif //SDRCOMMS_CLASSIFICATIONMODELPROCESSOR_H
