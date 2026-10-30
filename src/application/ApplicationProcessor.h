//
// Created by Chris Davey on 29/9/2026.
//

#ifndef SDRCOMMS_APPLICATIONPROCESSOR_H
#define SDRCOMMS_APPLICATIONPROCESSOR_H

#include "SampleProcessor.h"
#include "ProcessorOps.h"
#include "IQBlock.h"
#include <chrono>

#include "FFTSampleProcessor.h"
#include "MockProcessor.h"
#include "ModelFileLoader.h"
#include "SampleStreamerProcessor.h"

class ApplicationProcessor : public sdrcomms::SampleProcessor {
public:
    ApplicationProcessor(std::string udpServerAddress, int udpPort, double source_sample_rate_hz, std::string modelFile, std::string basePath);

    /**
     * initialise the application processor.
     * @return
     */
    bool init();

    /**
     * implement the process method
     * @param block
     */
    void process(sdrcomms::IQBlock &block) override;

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
    std::string basePath;
    std::string modelSpecFile;
    double source_sample_rate_hz = sdrcomms::DEFAULT_CENTER_FREQUENCY;
    sdrcomms::ModelFileSpec modelSpec {};
    std::vector<sdrcomms::ProcessorOps> processorOps {};

    std::unique_ptr<sdrcomms::SampleStreamerProcessor> sampleStreamerProcessor;

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
