//
// Created by Chris Davey on 1/10/2026.
//

#include "FFTSampleProcessor.h"
#include <memory>
namespace sdrcomms {
    FFTSampleProcessor::FFTSampleProcessor(std::string attach_key, size_t fftSize, std::shared_ptr<SampleProcessor> inner) {
        this->attach_key = attach_key;
        this->fftSize = fftSize;
        this->inner = inner;
        this->fft = std::make_shared<FFTTransform>();
        if (this->fft->init(fftSize)) {
            this->isInit.store(true, std::memory_order_relaxed);
        }
    }

    void FFTSampleProcessor::process(IQBlock &block) {
        if (this->isInit.load(std::memory_order_relaxed)) {
            // test processing the fft.
            std::vector<std::complex<float>> fftOut = this->fft->fft(block.samples);
            // attach the result
            // int channel, long long timestampMs, size_t sample_size, const std::vector<std::complex<float>> &samples
            std::shared_ptr<IQBlock> innerBlock = std::make_shared<IQBlock>(
                block.channel,
                block.timestampMs,
                fftOut.size(),
                std::move(fftOut));
            innerBlock->centerFreq = block.centerFreq;
            // we can add fft transformation as a processed attribute
            block.attachments.insert_or_assign(attach_key, innerBlock);

            if (this->inner != nullptr) {
                this->inner->process(*innerBlock);
            }
        }
    }
} // sdrcomms