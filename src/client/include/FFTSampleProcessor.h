//
// Created by Chris Davey on 1/10/2026.
//

#ifndef SDRCOMMS_FFTSAMPLEPROCESSOR_H
#define SDRCOMMS_FFTSAMPLEPROCESSOR_H
#include "SampleProcessor.h"
#include "FFTTransform.h"
#include <memory>
#include <atomic>
namespace sdrcomms {
    class FFTSampleProcessor : public SampleProcessor {
    public:
        FFTSampleProcessor(std::string attach_key, size_t fftSize, std::shared_ptr<SampleProcessor> inner );

        void process(IQBlock &block, std::map<std::string, std::shared_ptr<IQBlock>> &attachments) override;

    private:
        size_t fftSize;
        std::atomic<bool> isInit;
        std::shared_ptr<FFTTransform> fft;
        std::shared_ptr<SampleProcessor> inner;
    };
} // sdrcomms

#endif //SDRCOMMS_FFTSAMPLEPROCESSOR_H
