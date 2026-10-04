//
// Created by Chris Davey on 29/9/2026.
//

#include "include/NetIQBlock.h"
#include "include/BinaryIO.h"
namespace sdrcomms {

    NetIQBlock::NetIQBlock(const IQBlock& block) {
        this->timestampMs = block.timestampMs;
        this->sample_size = block.sample_size;
        this->centerFreq = block.centerFreq;
        this->channel = block.channel;
        this->iqValues.resize(block.samples.size());
        for (auto sample : block.samples) {
            this->iqValues.push_back( IQValue {sample.real(), sample.imag()});
        }
    }

    /**
         * convert the NetIQBlock to an IQBlock
         * @return iqblock
         */
    IQBlock NetIQBlock::toIQBlock() const {
        IQBlock block {};
        block.timestampMs = this->timestampMs;
        block.centerFreq = this->centerFreq;
        block.sample_size = this->sample_size;
        block.channel = this->channel;
        for (auto sample : this->iqValues) {
            block.samples.push_back(std::complex<float>(sample.real, sample.imag));
        }
        return block;
    }


    /**
     * serialize the block to stream.
     * @param out
     * @return
     */
    bool NetIQBlock::serialize(std::ostream& out) {
        BinaryIO::write_header(out);
        BinaryIO::write_pod(out, this->timestampMs);
        BinaryIO::write_pod(out, this->channel);
        BinaryIO::write_pod(out, this->centerFreq);
        BinaryIO::write_pod(out, this->sample_size);
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
        float centerFreq;
        BinaryIO::read_pod(in, ts);
        BinaryIO::read_pod(in, channel);
        BinaryIO::read_pod(in, centerFreq);
        BinaryIO::read_pod(in, sample_size);
        block.timestampMs = ts;
        block.sample_size = sample_size;
        block.centerFreq = centerFreq;
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

    /**
         * determine the byte size of a NetIQBlock
         * with a given sample size
         * @param sample_size
         * @return
         */
    size_t NetIQBlock::numBytes(size_t sample_size) {
        // header contains uint32_t MAGIC and VERSION
        size_t num_bytes = sizeof(uint32_t)*2;
        // timestampMs
        num_bytes += sizeof(long long);
        // channel
        num_bytes += sizeof(int);
        // centerfreq
        num_bytes += sizeof(float);
        // sample size
        num_bytes += sizeof(size_t);
        // actual vector size
        num_bytes += sizeof(size_t);
        // vector data
        num_bytes += sizeof(IQValue) * sample_size;
        return num_bytes;
    }

} // sdrcomms