//
// Created by Chris Davey on 26/10/2026.
//

#ifndef SDRCOMMS_DOWNSAMPLEPROCESSOR_H
#define SDRCOMMS_DOWNSAMPLEPROCESSOR_H
#include "SampleProcessor.h"
#include "SampleProcessor.h"
#include <memory>

namespace sdrcomms {
    class DownsampleProcessor : public SampleProcessor {
    public:
        /**
         * initialise with an attach key.
         * @param attach_key
         * @param target_samples_size
         * the target sample size needed for the classifier
         * @param source_rate_hz
         * @param target_rate_hz
         * @param inner
         */
        DownsampleProcessor(std::string attach_key, size_t target_samples_size, double source_rate_hz, double target_rate_hz, std::shared_ptr<SampleProcessor> inner);

        /**
         * process the incoming block and update the inner processor.
         * @param block
         */
        void process(IQBlock &block, std::map<std::string, std::shared_ptr<IQBlock>> &attachments) override;

    private:
        double source_rate_hz;
        double target_rate_hz;
        size_t target_samples_size;
        // the required number of samples before downsampling
        size_t required_input_samples;
        std::shared_ptr<IQBlock> accumulator;


        /**
         * downsampl using liquid-dsp
         * @param input_iq
         * @param downsampleRatio
         * @return
         */
        std::vector<std::complex<float>> downsample(const std::vector<std::complex<float>>& input_iq, float downsampleRatio, size_t targetOutputSize);

    };
} // sdrcomms

#endif //SDRCOMMS_DOWNSAMPLEPROCESSOR_H
