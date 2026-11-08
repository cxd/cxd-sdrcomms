//
// Created by Chris Davey on 20/9/2026.
//

#ifndef SDRCOMMS_IQBLOCK_H
#define SDRCOMMS_IQBLOCK_H
#include <vector>
#include <complex>
#include <map>
#include <memory>

namespace sdrcomms {
    class IQBlock {
    public:
        IQBlock() = default;
        IQBlock(size_t sample_size);
        IQBlock(int channel, long long timestampMs, size_t sample_size, const std::vector<std::complex<float>> &samples);
        size_t sample_size = 0;
        std::vector<std::complex<float>> samples {};
        int channel = 0;
        /**
         * time since epoch in milliseconds.
         */
        long long timestampMs = 0;
        double centerFreq = 0;

        /**
         * Attachments allow different transformations to be applied and attached to the block.
         * They are given a name, as the block is passed through the processing pipeline.
         * Nested processors can pass transformations by attaching them to the block.
         */
        std::map<std::string, std::shared_ptr<IQBlock>> attachments {};

    };
} // sdrcomms

#endif //SDRCOMMS_IQBLOCK_H
