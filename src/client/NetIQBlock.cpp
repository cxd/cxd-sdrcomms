//
// Created by Chris Davey on 29/9/2026.
//

#include "include/NetIQBlock.h"
#include "include/BinaryIO.h"

namespace sdrcomms {
    NetIQBlock::NetIQBlock(const IQBlock &block) {
        this->timestampMs = block.timestampMs;
        this->sample_size = block.samples.size();
        this->centerFreq = block.centerFreq;
        this->channel = block.channel;
        this->iqValues.resize(block.samples.size());
        this->num_samples = block.samples.size();
        for (int i = 0; i < block.samples.size(); i++) {
            std::complex<float> sample = block.samples[i];
            this->iqValues[i] = IQValue{
                .real = sample.real(),
                .imag = sample.imag()
            };
        }
    }

    /**
         * convert the NetIQBlock to an IQBlock
         * @return iqblock
         */
    IQBlock NetIQBlock::toIQBlock() const {
        IQBlock block{};
        block.timestampMs = this->timestampMs;
        block.centerFreq = this->centerFreq;
        block.sample_size = this->sample_size;
        block.channel = this->channel;
        block.samples.resize(this->iqValues.size());

        for (size_t i = 0; i < this->iqValues.size(); i++) {
            auto sample = this->iqValues[i];
            block.samples[i] = std::complex<float>(sample.real, sample.imag);
        }
        return block;
    }


    /**
     * serialize the block to stream.
     * @param out
     * @return
     * TODO: investigate serializing the iq values.
     */
    bool NetIQBlock::serialize(std::ostream &out) {
        BinaryIO::write_header(out);
        BinaryIO::write_pod(out, this->timestampMs);
        BinaryIO::write_pod(out, this->channel);
        BinaryIO::write_pod(out, this->centerFreq);
        BinaryIO::write_pod(out, this->sample_size);
        // num samples
        size_t numSamples = this->iqValues.size();
        this->num_samples = numSamples;
        BinaryIO::write_pod(out, numSamples);
        for (auto sample: this->iqValues) {
            sample.serialize(out);
        }

        return true;
    }

    /**
         * deserialize the block from the stream.
         * @param in
         * @return
         */
    NetIQBlock NetIQBlock::deserialize(std::istream &in) {
        NetIQBlock block{};
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
        // the number of samples to read.
        size_t numsamples;
        BinaryIO::read_pod(in, numsamples);
        block.num_samples = numsamples;
        block.iqValues.resize(numsamples);
        for (size_t i = 0; i < numsamples; i++) {
            const IQValue sample = IQValue::deserialize(in);
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
        size_t num_bytes = BinaryIO::header_byte_size();
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

    /**
         * read only the header
         * @param in
         * @return
         */
    NetIQBlock NetIQBlock::read_header(std::istream &in) {
        NetIQBlock block{};
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
        block.num_samples = numsamples;
        return block;
    }


    /**
     * get the size of the header
     * @return
     */
    size_t NetIQBlock::headerSize() {
        // header contains uint32_t MAGIC and VERSION
        size_t num_bytes = sizeof(uint32_t) * 2;
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
        return num_bytes;
    }
} // sdrcomms
