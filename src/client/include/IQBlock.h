//
// Created by Chris Davey on 20/9/2026.
//

#ifndef SDRCOMMS_IQBLOCK_H
#define SDRCOMMS_IQBLOCK_H
#include <vector>
#include <complex>

namespace sdrcomms {
    class IQBlock {
    public:
        IQBlock() = default;
        IQBlock(size_t sample_size);
        size_t sample_size;
        std::vector<std::complex<float>> samples {};
        double centerFreq;
    };
} // sdrcomms

#endif //SDRCOMMS_IQBLOCK_H
