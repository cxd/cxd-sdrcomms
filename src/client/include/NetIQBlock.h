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
         * convert the NetIQBlock to an IQBlock
         * @return iqblock
         */
        IQBlock toIQBlock() const;

        /**
         * deserialize the block from the stream.
         * @param in
         * @return
         */
        static NetIQBlock deserialize(std::istream& in);

        /**
         * read only the header
         * @param in
         * @return
         */
        static NetIQBlock read_header(std::istream& in);

        /**
         * determine the byte size of a NetIQBlock
         * with a given sample size
         * @param sample_size
         * @return
         */
        static size_t numBytes(size_t sample_size);

        /**
         * get the size of the header
         * @return
         */
        static size_t headerSize();

        /**
         * use public fields
         */
        long long timestampMs;
        float centerFreq;
        int channel;
        size_t sample_size;
        // actual number of samples.
        size_t num_samples;
        std::vector<IQValue> iqValues;

    };
} // sdrcomms

#endif //SDRCOMMS_NETIQBLOCK_H
