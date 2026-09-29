//
// Created by Chris Davey on 29/9/2026.
//

#include "include/NetIQBlock.h"
#include "include/BinaryIO.h"
namespace sdrcomms {

    NetIQBlock::NetIQBlock(const IQBlock& block) {
        this->timestampMs = block.timestampMs;
        this->sample_size = block.sample_size;
        this->channel = block.channel;
        this->iqValues.resize(block.samples.size());
        for (auto sample : block.samples) {
            this->iqValues.push_back( IQValue {sample.real(), sample.imag()});
        }
    }

    /**
     * serialize the block to stream.
     * @param out
     * @return
     */
    bool NetIQBlock::serialize(std::ostream& out) {
        BinaryIO::write_header(out);
        BinaryIO::write_pod(out, this->timestampMs);
        BinaryIO::write_pod(out, this->sample_size);
        BinaryIO::write_pod(out, this->channel);
        BinaryIO::write_pod(out, this->iqValues.size());
        for (auto sample : this->iqValues) {
            BinaryIO::write_pod(out, sample);
        }

        return true;
    }

    /**
         * deserialize the block from the stream.
         * @param in
         * @return
         */
    NetIQBlock NetIQBlock::deserialize(std::istream& in) {
        NetIQBlock block {};
        BinaryIO::read_header(in);
        long long ts;
        size_t sample_size;
        int channel;
        BinaryIO::read_pod(in, ts);
        BinaryIO::read_pod(in, sample_size);
        BinaryIO::read_pod(in, channel);
        block.timestampMs = ts;
        block.sample_size = sample_size;
        block.channel = channel;
        size_t numsamples;
        BinaryIO::read_pod(in, numsamples);
        block.iqValues.resize(numsamples);
        for (size_t i=0; i<numsamples;i++) {
            IQValue sample;
            BinaryIO::read_pod(in, sample);
            block.iqValues[i] = sample;
        }

        return block;
    }
} // sdrcomms