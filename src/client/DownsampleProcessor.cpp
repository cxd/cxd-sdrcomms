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
    DownsampleProcessor::DownsampleProcessor(std::string attach_key, double source_rate_hz, double target_rate_hz, std::shared_ptr<SampleProcessor> inner) {
        this->attach_key = attach_key;
        this->source_rate_hz = source_rate_hz;
        this->target_rate_hz = target_rate_hz;
        this->inner = inner;
    }

    /**
     * process the incoming block and update the inner processor.
     * @param block
     */
    void DownsampleProcessor::process(IQBlock &block) {
        std::vector<std::complex<float>> output_iq = downsample(block.samples, target_rate_hz/source_rate_hz);
        // attach the result
        // int channel, long long timestampMs, size_t sample_size, const std::vector<std::complex<float>> &samples
        std::shared_ptr<IQBlock> innerBlock = std::make_shared<IQBlock>(
            block.channel,
            block.timestampMs,
            output_iq.size(),
            std::move(output_iq));
        innerBlock->centerFreq = block.centerFreq;
        // we can add fft transformation as a processed attribute
        block.attachments.insert_or_assign(attach_key, innerBlock);

        if (this->inner != nullptr) {
            this->inner->process(*innerBlock);
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