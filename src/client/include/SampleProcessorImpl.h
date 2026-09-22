//
// Created by Chris Davey on 22/9/2026.
//

#ifndef SDRCOMMS_SAMPLEPROCESSORIMPL_H
#define SDRCOMMS_SAMPLEPROCESSORIMPL_H
#include "SampleProcessor.h"

namespace sdrcomms {
    class SampleProcessorImpl : public SampleProcessor {
    public:
        void process(const IQBlock &block) override;
    };
} // sdrcomms

#endif //SDRCOMMS_SAMPLEPROCESSORIMPL_H
