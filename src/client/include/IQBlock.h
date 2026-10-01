//
// Created by Chris Davey on 20/9/2026.
//

#ifndef SDRCOMMS_IQBLOCK_H
#define SDRCOMMS_IQBLOCK_H
#include <vector>
#include <complex>
#include <memory>

namespace sdrcomms {
    class IQBlock {
    public:
        IQBlock() = default;
        IQBlock(size_t sample_size);
        IQBlock(int channel, long long timestampMs, size_t sample_size, const std::vector<std::complex<float>> &samples);
        size_t sample_size;
        std::vector<std::complex<float>> samples {};
        int channel;
        /**
         * time since epoch in milliseconds.
         */
        long long timestampMs;
        double centerFreq;

        /**
         * an inner block can be passed referenced by the outer block to support
         * collection of transformations.
         * The inner block is not sent over the network.
         */
        std::shared_ptr<IQBlock> innerBlock = nullptr;

    };
} // sdrcomms

#endif //SDRCOMMS_IQBLOCK_H
