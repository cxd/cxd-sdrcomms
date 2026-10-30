//
// Created by Chris Davey on 26/10/2026.
//

#include "include/DownsampleProcessor.h"
#include <liquid/liquid.h>

namespace sdrcomms {
    /**
         * initialise with an attach key.
         * @param attach_key
         * @param source_rate_hz
         * @param target_rate_hz
         * @param inner
         */
    DownsampleProcessor::DownsampleProcessor(std::string attach_key, size_t target_samples_size, double source_rate_hz,
                                             double target_rate_hz, std::shared_ptr<SampleProcessor> inner) {
        this->attach_key = attach_key;
        this->source_rate_hz = source_rate_hz;
        this->target_rate_hz = target_rate_hz;
        this->inner = inner;
        this->accumulator = nullptr;
        this->target_samples_size = target_samples_size;
        this->required_input_samples = static_cast<size_t>(std::ceil(
            target_samples_size / (target_rate_hz / source_rate_hz)));
    }

    /**
     * process the incoming block and update the inner processor.
     *
     * This processor will need to batch accumulate the samples until it has enough samples
     * to perform the downsampling operation and obtain samples at the desired sample rate.
     *
     * Internally it maintains a batch queue to allow accumulation of a required sample size.
     * We need to collect input samples until we have enough to perform the downsampling operation.
     * $ input_n = out_n / r $
     *
     * where r is the downsample ratio
     *
     * for block samples of 65536
     * if we have r = 0.002
     * and target output size of 2048
     * $ input_n = 2048 / 0.002 = 1024000.0 $
     * This does not evenly divide into 65536, so this means we are collecting
     * $$ 1024000 / 65536 = 15.625 $$ blocks before we can run the downsampling operation.
     * The remainder of the last whole block needs to be added to the front of the accumulator.
     * @param block
     */
    void DownsampleProcessor::process(IQBlock &block, std::map<std::string, std::shared_ptr<IQBlock>> &attachments) {
        // initialise the first pointer
        if (accumulator == nullptr) {
            accumulator = std::make_shared<IQBlock>(block);
            return;
        }
        if (accumulator->samples.size() + block.samples.size() >= required_input_samples) {
            // add the block samples to the accumulator
            accumulator->samples.insert(accumulator->samples.end(), block.samples.begin(), block.samples.end());
            // we can perform resampling here.
            // extract the required range of samples
            size_t delta = accumulator->samples.size() - required_input_samples;
            std::vector<std::complex<float> > samples{};
            samples.insert(samples.begin(), accumulator->samples.begin(), accumulator->samples.end() - delta);
            float downsampleRatio = target_rate_hz / source_rate_hz;
            std::vector<std::complex<float> > output_iq = downsample(samples, downsampleRatio, target_samples_size);
            if (output_iq.size() == 0) {
                accumulator = nullptr;
                return;
            }
            // attach the result
            // int channel, long long timestampMs, size_t sample_size, const std::vector<std::complex<float>> &samples
            std::shared_ptr<IQBlock> innerBlock = std::make_shared<IQBlock>(
                block.channel,
                // assign the timestamp to the accumulator timestamp
                accumulator->timestampMs,
                output_iq.size(),
                std::move(output_iq));
            innerBlock->centerFreq = block.centerFreq;
            // previous attachments
            //
            // we can add fft transformation as a processed attribute
            attachments.insert_or_assign(attach_key, innerBlock);

            // reset the accumulator timestamp to the block timestamp.
            std::vector<std::complex<float> > subset{};
            if (delta > 0) {
                subset.insert(subset.begin(), accumulator->samples.end() - delta, accumulator->samples.end());
            }
            // copy the remaining subset of the data into the next accumulator.
            this->accumulator = std::make_shared<IQBlock>(block.channel,
                                                          block.timestampMs,
                                                          block.sample_size,
                                                          subset);

            if (this->inner != nullptr) {
                this->inner->process(*innerBlock, attachments);
            }
        } else {
            accumulator->samples.insert(accumulator->samples.end(), block.samples.begin(), block.samples.end());
        }
    }

    /**
         * downsampl using liquid-dsp
         * @param input_iq
         * @param downsampleRatio
         * @return
         */
    std::vector<std::complex<float> > DownsampleProcessor::downsample(const std::vector<std::complex<float> > &input_iq,
                                                                      float downsampleRatio,
                                                                      size_t targetOutputSize) {
        size_t decimation_factor = static_cast<size_t>(std::round(1.0f / downsampleRatio)); // e.g. 4000

        // Design a low-pass filter to prevent aliasing at the new low rate
        // A standard filter length of 64 or 128 coefficients is plenty for clean rejection
        unsigned int h_len = 129;
        std::vector<float> h(h_len);

        // liquid-dsp helper to create lowpass coefficients on the fly
        // Cutoff frequency is half of our target downsampled bandwidth
        liquid_firdes_kaiser(h_len, downsampleRatio * 0.5f, 60.0f, 0.0f, h.data());

        std::vector<std::complex<float>> output_iq;
        output_iq.reserve(targetOutputSize);

        // Calculate the structural delay of the filter coefficients
        size_t filter_delay = (h_len - 1) / 2;

        // Apply the filter only at the exact decimated steps (Polyphase style)
        for (size_t i = filter_delay; i < input_iq.size() && output_iq.size() < targetOutputSize; i += decimation_factor) {
            std::complex<float> filtered_sample(0.0f, 0.0f);

            // Convolve the filter window over the input
            for (unsigned int g = 0; g < h_len; ++g) {
                size_t idx = i + g - filter_delay;
                if (idx < input_iq.size()) {
                    filtered_sample += input_iq[idx] * h[g];
                }
            }
            output_iq.push_back(filtered_sample);
        }

        // Force perfect sizing match for the classifier
        if (output_iq.size() < targetOutputSize) {
            output_iq.resize(targetOutputSize, {0.0f, 0.0f});
        }

        return output_iq;
    }
} // sdrcomms
