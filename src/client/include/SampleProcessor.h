//
// Created by Chris Davey on 22/9/2026.
//

#ifndef SDRCOMMS_SAMPLEPROCESSOR_H
#define SDRCOMMS_SAMPLEPROCESSOR_H
#include "IQBlock.h"

namespace sdrcomms {
    class SampleProcessor {
    public:
        /**
         * process the supplied IQBlock
         * @param block
         */
        void process(const IQBlock &block) = 0;
    };
}


#endif //SDRCOMMS_SAMPLEPROCESSOR_H
