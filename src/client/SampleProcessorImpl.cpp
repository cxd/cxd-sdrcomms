//
// Created by Chris Davey on 22/9/2026.
//

#include "include/SampleProcessorImpl.h"
#include<iostream>

namespace sdrcomms {
    SampleProcessorImpl::SampleProcessorImpl(std::shared_ptr<SampleProcessor> inner) {
        this->inner = inner;
    }

    /**
     * process the supplied IQBlock
     * @param block
     */
    void SampleProcessorImpl::process(const IQBlock &block) {
        if (inner != nullptr) {
            try {
                inner->process(block);
            } catch (std::exception &e) {
                std::cerr << "Exception caught in SampleProcessorImpl: " << e.what() << std::endl;
            }
        }
    }
} // sdrcomms
