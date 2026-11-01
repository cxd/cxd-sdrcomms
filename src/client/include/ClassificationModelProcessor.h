//
// Created by Chris Davey on 30/10/2026.
//

#ifndef SDRCOMMS_CLASSIFICATIONMODELPROCESSOR_H
#define SDRCOMMS_CLASSIFICATIONMODELPROCESSOR_H
#include <thread>

#include "ClassMap.h"
#include "ControlMessage.h"
#include "MessageQueue.h"
#include "ModelFileLoader.h"
#include "ModelFileSpec.h"
#include "ProcessorOps.h"
#include "RootMeanSquareProcessor.h"
#include "SampleProcessor.h"

namespace sdrcomms {
    class ClassificationModelProcessor : public SampleProcessor {
    public:
        ClassificationModelProcessor(std::string attach_key,
            std::string basePath,
            const ModelFileSpec &modelSpec,
            const ClassMap &classMap,
            std::string rms_db_key,
            float rms_db_threshold,
            std::shared_ptr<RootMeanSquareProcessor> rmsProcessor,
            const std::vector<ProcessorOps> &processorOps,
            std::shared_ptr<MessageQueue<sdrcomms::ControlMessage>> eventQueue);

        /**
         * initialise the processor
         * @return
         */
        bool init();

        /**
         * stop classification process
         */
        void teardown() override;

        /**
         * process the incoming block and update the inner processor.
         * @param block
         */
        void process(IQBlock &block, std::map<std::string, std::shared_ptr<IQBlock>> &attachments) override;

    private:
        bool isInit = false;
        std::string basePath;
        std::string modelPath;

        // the key to lookup the rms db threshold
        std::string rms_db_key = "rms_db";
        // set the rms db threshold for signal detection
        // we ignore any signals with a lower threshold.
        float rms_db_threshold = -30;
        std::shared_ptr<RootMeanSquareProcessor> rmsProcessor;

        ModelFileSpec modelSpec{};
        ClassMap classMap{};
        std::vector<ProcessorOps> processorOps;
        std::unique_ptr<ModelFileLoader> modelFileLoader;
        std::shared_ptr<sdrcomms::MessageQueue<sdrcomms::ControlMessage>> eventQueue;
        /**
         * incoming queue of blocks to classify perform classification asynchronously
         */
        std::shared_ptr<MessageQueue<IQBlock>> classifyQueue;
        std::thread classifyThread;
        // internal flag indicating if streaming is active.
        std::atomic<bool> isRunning;


        /**
         * initialise the model.
         * @return
         */
        bool load_model();


        /**
         * classification worker thread
         */
        void classify_worker();

        /**
         * lookup rms db
         */
        float get_rms_db_threshold(std::map<std::string, std::shared_ptr<IQBlock>> &attachments);
    };
} // sdrcomms

#endif //SDRCOMMS_CLASSIFICATIONMODELPROCESSOR_H
