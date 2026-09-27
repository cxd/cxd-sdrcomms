//
// Created by Chris Davey on 22/9/2026.
//

#ifndef SDRCOMMS_SAMPLEPROCESSORIMPL_H
#define SDRCOMMS_SAMPLEPROCESSORIMPL_H

#include <memory>
#include "SampleProcessor.h"

namespace sdrcomms {
    class SampleProcessorImpl : public SampleProcessor {
    public:

        SampleProcessorImpl(std::shared_ptr<SampleProcessor> inner);

        void process(const IQBlock &block) override;

    private:
        std::shared_ptr<SampleProcessor> inner;
    };
} // sdrcomms

#endif //SDRCOMMS_SAMPLEPROCESSORIMPL_H
