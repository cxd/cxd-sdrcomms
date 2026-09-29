//
// Created by Chris Davey on 20/9/2026.
//

#include "include/ReceiveQueue.h"

namespace sdrcomms {
    ReceiveQueue::ReceiveQueue(size_t capacity, size_t sample_size) {
        this->capacity = capacity;
        this->sample_size = sample_size;
        this->blocks.resize(capacity);
        // need to initialise blocks.
        for (auto &block : this->blocks) {
            block = IQBlock(sample_size);
        }
    }

    bool ReceiveQueue::push(long long timestamp, int channel, const std::vector<std::complex<float>>& data, size_t size, double freq) {
        size_t current_write = write_index.load(std::memory_order_relaxed);
        size_t next_write = (current_write + 1 % capacity);
        if (next_write >= blocks.size() - 1) {
            next_write = 0;
        }
        // check if the next write is equal to the read index.
        if (next_write == read_index.load(std::memory_order_relaxed)) {
            return false;
        }

        // vector copy samples.
        blocks[next_write].timestampMs = timestamp;
        blocks[next_write].channel = channel;
        blocks[next_write].sample_size = size;
        blocks[next_write].samples = std::move(data);
        blocks[next_write].centerFreq = freq;

        // update the current write index.
        write_index.store(next_write, std::memory_order_relaxed);
        return true;
    }

    bool ReceiveQueue::pop(IQBlock &dest_block) {
        // copy the IQ block into the return block and update the read index
        size_t current_read = read_index.load(std::memory_order_relaxed);
        size_t current_write = write_index.load(std::memory_order_relaxed);

        // if the read matches the write position the queue is empty.
        if (current_read == current_write) {
            return false;
        }

        // deep copy the data from the queue into the supplied reference.
        dest_block.sample_size = blocks[current_read].sample_size;
        dest_block.samples.resize(blocks[current_read].sample_size);
        dest_block.samples = blocks[current_read].samples;
        dest_block.centerFreq = blocks[current_read].centerFreq;

        // move the read position
        size_t next_read = (current_read + 1) % capacity;
        if (next_read >= blocks.size() - 1) {
            next_read = 0;
        }
        read_index.store(next_read, std::memory_order_relaxed);
        return true;
    }
}
