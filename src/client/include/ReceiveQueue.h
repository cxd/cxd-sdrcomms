//
// Created by Chris Davey on 20/9/2026.
//

#ifndef SDRCOMMS_RECEIVEQUEUE_H
#define SDRCOMMS_RECEIVEQUEUE_H
#include <atomic>
#include <vector>

#include "IQBlock.h"

namespace sdrcomms {
    class ReceiveQueue {
    public:
        ReceiveQueue(size_t capacity, size_t sample_size);

        bool push(const std::vector<std::complex<float>>& data, size_t size, double freq);

        bool pop(IQBlock& dest_block);

    private:
        std::vector<IQBlock> blocks {};
        size_t capacity;
        size_t sample_size;
        std::atomic<size_t> write_index{0};
        std::atomic<size_t> read_index{0};
    };
}


#endif //SDRCOMMS_RECEIVEQUEUE_H
