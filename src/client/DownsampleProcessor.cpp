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
    DownsampleProcessor::DownsampleProcessor(std::string attach_key, size_t input_samples_size, double source_rate_hz, double target_rate_hz, std::shared_ptr<SampleProcessor> inner) {
        this->attach_key = attach_key;
        this->source_rate_hz = source_rate_hz;
        this->target_rate_hz = target_rate_hz;
        this->inner = inner;
        this->accumulator = nullptr;
        this->input_sample_size = input_samples_size;
        this->required_input_samples = static_cast<size_t>(std::ceil(
            input_samples_size / (source_rate_hz / target_rate_hz)));
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
    void DownsampleProcessor::process(IQBlock &block) {
        // initialise the first pointer
        if (accumulator == nullptr) {
            accumulator = std::make_shared<IQBlock>(block);
            return;
        } else if (accumulator->samples.size() + block.samples.size() >= input_sample_size) {
            // add the block samples to the accumulator
            accumulator->samples.insert(accumulator->samples.end(), block.samples.begin(), block.samples.end());
            // we can perform resampling here.
            // extract the required range of samples
            size_t delta = accumulator->samples.size() - input_sample_size;
            std::vector<std::complex<float>> samples {};
            if (delta > 0) {
                samples.insert(samples.begin(), accumulator->samples.begin(), accumulator->samples.end() - delta);
            }
            std::vector<std::complex<float>> output_iq = downsample(samples, target_rate_hz/source_rate_hz);
            // attach the result
            // int channel, long long timestampMs, size_t sample_size, const std::vector<std::complex<float>> &samples
            std::shared_ptr<IQBlock> innerBlock = std::make_shared<IQBlock>(
                block.channel,
                // assign the timestamp to the accumulator timestamp
                accumulator->timestampMs,
                output_iq.size(),
                std::move(output_iq));
            innerBlock->centerFreq = block.centerFreq;
            // we can add fft transformation as a processed attribute
            block.attachments.insert_or_assign(attach_key, innerBlock);

            // reset the accumulator timestamp to the block timestamp.
            std::vector<std::complex<float>> subset {};
            if (delta > 0) {
                subset.insert(subset.begin(), accumulator->samples.end() - delta, accumulator->samples.end());
            }
            // copy the remaining subset of the data into the next accumulator.
            this->accumulator = std::make_shared<IQBlock>(block.channel,
                block.timestampMs,
                block.sample_size,
                subset);

            if (this->inner != nullptr) {
                this->inner->process(*innerBlock);
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
    std::vector<std::complex<float>> DownsampleProcessor::downsample(const std::vector<std::complex<float>>& input_iq, float downsampleRatio) {
        // Asymmetry parameter (stop-band attenuation in dB).
        // 60dB reduces out-of-band aliasing noise by a factor of 1,000.
        // 80dB is excellent for high-fidelity audio/radio environments.
        float stop_band_attenuation = 60.0f;
        // initialise multistage resampler
        // 'crcf' stands for: Complex input, Real coefficients, Complex output, Floating point
        msresamp_crcf resampler = msresamp_crcf_create(downsampleRatio, stop_band_attenuation);
        float delay = msresamp_crcf_get_delay(resampler);

        /**
         * input_n = out_n / r
         *
         * where r is the downsample ratio
         */

        // allocate a vector slightly larger than the actual target size
        size_t target_size = static_cast<unsigned int>(std::lround(input_iq.size() * downsampleRatio));

        size_t delay_samples = static_cast<size_t>(std::ceil(delay/downsampleRatio) + 1);
        std::vector<std::complex<float>> input_copy(delay_samples + input_iq.size(), {0.0f, 0.0f});
        // append to the beginning of the vector
        std::copy(input_iq.begin(), input_iq.end(), input_copy.begin());

        // liquid recommends at least ceil(1 + 2 * r * nx) output samples
        size_t max_output_size = static_cast<size_t>(std::ceil(1 + 2 * downsampleRatio * input_copy.size()));
        std::vector<std::complex<float>> output_iq(max_output_size);


        // Because std::complex<float> is guaranteed by the C++ standard to be layout-compatible
        // with a two-element float array [real, imag], we can safely perform a zero-copy pointer cast.
        const liquid_float_complex* type_safe_input = reinterpret_cast<const liquid_float_complex*>(input_copy.data());
        liquid_float_complex* type_safe_output = reinterpret_cast<liquid_float_complex*>(output_iq.data());

        unsigned int num_samples_written = 0;
        // the time-domain polyphase processing loop
        msresamp_crcf_execute(resampler,
                              const_cast<liquid_float_complex*>(type_safe_input),
                              input_iq.size(),
                              type_safe_output,
                              &num_samples_written);
        // Clean up the internal filter object memory handles
        msresamp_crcf_destroy(resampler);
        // drop the filter start-up transient so the output aligns with the input,
        // then cut the flushed tail back to the expected length.
        size_t skip = std::min(static_cast<size_t>(std::lround(delay)), output_iq.size());
        output_iq.erase(output_iq.begin(), output_iq.begin() + skip);
        if (output_iq.size() > target_size) {
            output_iq.resize(target_size);
        }
        return output_iq;
    }

} // sdrcomms