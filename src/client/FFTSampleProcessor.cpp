//
// Created by Chris Davey on 1/10/2026.
//

#include "FFTSampleProcessor.h"
#include <memory>
namespace sdrcomms {
    FFTSampleProcessor::FFTSampleProcessor(size_t fftSize, std::shared_ptr<SampleProcessor> inner) {
        this->fftSize = fftSize;
        this->inner = inner;
        this->fft = std::make_shared<FFTTransform>();
        if (this->fft->init(fftSize)) {
            this->isInit.store(true, std::memory_order_relaxed);
        }
    }

    void FFTSampleProcessor::process(const IQBlock &block) {
        if (this->isInit.load(std::memory_order_relaxed)) {
            // test processing the fft.
            std::vector<std::complex<float>> fftOut = this->fft->fft(block.samples);
            if (this->inner != nullptr) {
                IQBlock innerBlock;
                innerBlock.centerFreq = block.centerFreq;
                innerBlock.channel = block.channel;
                innerBlock.sample_size = fftOut.size();
                innerBlock.samples = fftOut;
                // we can add fft transformation as a processed attribute
                innerBlock.attachments["fft"] = std::make_shared<IQBlock>( block );
                this->inner->process(innerBlock);
            }
        }
    }
} // sdrcomms