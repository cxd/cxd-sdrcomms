//
// Created by Chris Davey on 29/9/2026.
//

#ifndef SDRCOMMS_NETIQBLOCK_H
#define SDRCOMMS_NETIQBLOCK_H
#include "IQBlock.h"
#include <ostream>
#include <vector>

#include "BinaryIO.h"

namespace sdrcomms {
    /**
     * an IQ value for serialization.
     */
    struct IQValue {
        float real;
        float imag;
    };

    class NetIQBlock {
    public:

        NetIQBlock() = default;

        NetIQBlock(const IQBlock& block);

        /**
         * serialize the block to stream.
         * @param out
         * @return
         */
        bool serialize(std::ostream& out);

        /**
         * deserialize the block from the stream.
         * @param in
         * @return
         */
        static NetIQBlock deserialize(std::istream& in);

        /**
         * use public fields
         */
        long long timestampMs;
        int channel;
        size_t sample_size;
        std::vector<IQValue> iqValues;

    };
} // sdrcomms

#endif //SDRCOMMS_NETIQBLOCK_H
