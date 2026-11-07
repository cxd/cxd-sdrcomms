//
// Created by Chris Davey on 11/10/2026.
//

#ifndef SDRCOMMS_MESSAGEQUEUE_H
#define SDRCOMMS_MESSAGEQUEUE_H
#include <atomic>
#include <cstddef>
#include <vector>

namespace sdrcomms {
    template<typename T>
    class MessageQueue {
    public:
        MessageQueue(size_t qsize) {
            if (qsize <= 2) {
                throw std::invalid_argument("Queue size must be greater than 2");
            }
            capacity = qsize;
            write_index.store(0, std::memory_order_relaxed);
            read_index.store(0, std::memory_order_relaxed);
            blocks.resize(qsize);
            for (size_t i = 0; i < blocks.size(); i++) {
                blocks[i] = T();
            }
        }

        bool push(T &item) {
            size_t current_write = write_index.load(std::memory_order_relaxed);
            size_t next_write = (current_write + 1) % capacity;

            // check if the next write is equal to the read index.
            if (next_write == read_index.load(std::memory_order_acquire)) {
                return false;
            }

            // vector copy samples.
            blocks[current_write] = std::move(item);

            // update the current write index.
            write_index.store(next_write, std::memory_order_release);
            return true;
        }

        bool pop(T &item) {
            // copy the IQ block into the return block and update the read index
            size_t current_read = read_index.load(std::memory_order_relaxed);

            // if the read matches the write position the queue is empty.
            if (current_read == write_index.load(std::memory_order_acquire)) {
                return false;
            }
            // move the current object into the reference.
            item = std::move(blocks[current_read]);

            // move the read position
            size_t next_read = (current_read + 1) % capacity;

            read_index.store(next_read, std::memory_order_release);
            return true;
        }

    private:
        std::vector<T> blocks{};
        size_t capacity;
        std::atomic<size_t> write_index{0};
        std::atomic<size_t> read_index{0};
    };
} // sdrcomms

#endif //SDRCOMMS_MESSAGEQUEUE_H
