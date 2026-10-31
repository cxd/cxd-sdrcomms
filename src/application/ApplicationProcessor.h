//
// Created by Chris Davey on 29/9/2026.
//

#ifndef SDRCOMMS_APPLICATIONPROCESSOR_H
#define SDRCOMMS_APPLICATIONPROCESSOR_H

#include "SampleProcessor.h"
#include "ProcessorOps.h"
#include "IQBlock.h"
#include <chrono>

#include "ClassificationModelProcessor.h"
#include "ClassMap.h"
#include "ControlMessage.h"
#include "FFTSampleProcessor.h"
#include "MessageQueue.h"
#include "MockProcessor.h"
#include "ModelFileLoader.h"
#include "SampleStreamerProcessor.h"

class ApplicationProcessor : public sdrcomms::SampleProcessor {
public:
    ApplicationProcessor(std::string udpServerAddress,
                         int udpPort,
                         double source_sample_rate_hz,
                         std::string modelFile,
                         std::string classMapFilename,
                         std::string basePath,
                         std::shared_ptr<sdrcomms::MessageQueue<sdrcomms::ControlMessage> > eventQueue);

    /**
     * initialise the application processor.
     * @return
     */
    bool init();

    /**
     * tear down the processor chain
     */
    void teardown() override;

    /**
     * implement the process method
     * @param block
     */
    void process(sdrcomms::IQBlock &block,
                 std::map<std::string, std::shared_ptr<sdrcomms::IQBlock> > &attachments) override;

    /**
     * expose start server
     */
    void startServer();

    /**
     * expose stop server
     */
    void stopServer();

private:
    std::string udpServerAddress;
    int udpPort;
    static const long ONE_SECOND = 1000;

    std::string modelPath;
    std::string classMapFilename;
    std::string basePath;
    std::string modelSpecFile;
    double source_sample_rate_hz = sdrcomms::DEFAULT_CENTER_FREQUENCY;

    std::shared_ptr<sdrcomms::MessageQueue<sdrcomms::ControlMessage> > eventQueue;

    sdrcomms::ModelFileSpec modelSpec{};
    sdrcomms::ClassMap classMap{};
    std::vector<sdrcomms::ProcessorOps> processorOps{};

    std::unique_ptr<sdrcomms::SampleStreamerProcessor> sampleStreamerProcessor;
    std::shared_ptr<sdrcomms::ClassificationModelProcessor> modelProcessor;
    std::shared_ptr<sdrcomms::SampleProcessor> processingChain;

    long blocks_per_second;
    std::chrono::time_point<std::chrono::system_clock> last_packet;

    /**
     * initialise the model.
     * @return
     */
    bool load_model();

    /**
     * get the processor operations from the model spec.
     * @return
     */
    std::vector<sdrcomms::ProcessorOps> get_operations();

    /**
     * initialise the model processor.
     * @return
     */
    std::shared_ptr<sdrcomms::SampleProcessor> make_model_processor();

    /**
     * build the processing chain.
     */
    std::shared_ptr<sdrcomms::SampleProcessor> build_processing_chain(std::shared_ptr<SampleProcessor> lastProcessor);
};


#endif //SDRCOMMS_APPLICATIONPROCESSOR_H
