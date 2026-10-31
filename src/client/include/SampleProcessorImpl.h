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

        SampleProcessorImpl(std::string attach_key, std::shared_ptr<SampleProcessor> inner);

        void process(IQBlock &block, std::map<std::string, std::shared_ptr<IQBlock>> &attachments) override;

    };
} // sdrcomms

#endif //SDRCOMMS_SAMPLEPROCESSORIMPL_H
