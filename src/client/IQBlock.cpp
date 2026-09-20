//
// Created by Chris Davey on 20/9/2026.
//

#include "include/IQBlock.h"

namespace sdrcomms {

    IQBlock::IQBlock(size_t sample_size) {
        this->sample_size = sample_size;
        this->samples.resize(sample_size);
        for (int i=0;i<sample_size;i++) {
            this->samples[i] = std::complex<float>(0,0);
        }
    }
} // sdrcomms