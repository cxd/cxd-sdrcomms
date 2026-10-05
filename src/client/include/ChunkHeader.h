//
// Created by Chris Davey on 5/10/2026.
//

#ifndef SDRCOMMS_CHUNKHEADER_H
#define SDRCOMMS_CHUNKHEADER_H
#include <cstdint>
#include <ostream>
#include <istream>


namespace sdrcomms {
    class ChunkHeader {
    public:
        uint32_t chunk_index;
        uint32_t chunk_count;
        size_t total_bytes;

        ChunkHeader() = default;

        ChunkHeader(uint32_t chunk_index, uint32_t chunk_count, size_t total_bytes) : chunk_index(chunk_index),
            chunk_count(chunk_count),
            total_bytes(total_bytes) {
        }


        /**
         * get the number of bytes in the header.
         * @return
         */
        static size_t getBytesize();

        /**
         * get the expected payload byte size.
         * @return
         */
        size_t getPayloadByteSize();

        /**
         * serialize the header to ouput stream
         * @param os
         * @return
         */
        bool serialize(std::ostream &os) const;

        /**
         * deserialize the header.
         * @param is
         * @return
         */
        static ChunkHeader deserialize(std::istream &is);
    };
} // sdrcomms

#endif //SDRCOMMS_CHUNKHEADER_H
