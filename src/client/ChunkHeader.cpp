//
// Created by Chris Davey on 5/10/2026.
//

#include "include/ChunkHeader.h"
#include "include/BinaryIO.h"


namespace sdrcomms {
    /**
     * get the number of bytes in the header.
     * @return
     */
    size_t ChunkHeader::getBytesize() {
        size_t byte_size = BinaryIO::header_byte_size();
        byte_size += sizeof(uint32_t)*3;
        return byte_size;
    }

    /**
         * get the expected payload byte size.
         * In the first index, the payload itself is
         * prefixed by a binaryio header.
         * @return
         */
    size_t ChunkHeader::getPayloadByteSize() {
        size_t byte_size = BinaryIO::header_byte_size();
        if (this->chunk_index == 0) {
            return this->total_bytes + byte_size;
        } else {
            return this->total_bytes;
        }
    }


    /**
     * serialize the header to ouput stream
     * @param os
     * @return
     */
    bool ChunkHeader::serialize(std::ostream& out) const {
        BinaryIO::write_header(out);
        BinaryIO::write_pod(out, this->chunk_count);
        BinaryIO::write_pod(out, this->chunk_index);
        if (this->chunk_index == 0) {
            // the inner payload is prefixed by a binaryio header as well
            size_t byte_size = BinaryIO::header_byte_size();
            byte_size += this->total_bytes;
            uint64_t temp_size = static_cast<uint64_t>(byte_size);
            BinaryIO::write_pod(out, temp_size);
        } else {
            uint64_t temp_size = static_cast<uint64_t>(total_bytes);
            BinaryIO::write_pod(out, temp_size);
        }
        return true;
    }

    /**
     * deserialize the header.
     * @param is
     * @return
     */
    ChunkHeader ChunkHeader::deserialize(std::istream& in) {
        BinaryIO::read_header(in);
        uint32_t chunk_count;
        BinaryIO::read_pod(in, chunk_count);
        uint32_t chunk_index;
        BinaryIO::read_pod(in, chunk_index);
        size_t total_bytes;
        uint64_t temp_bytes;
        BinaryIO::read_pod(in, temp_bytes);
        total_bytes = static_cast<size_t>(temp_bytes);
        if (chunk_index == 0) {
            size_t byte_size = BinaryIO::header_byte_size();
            total_bytes -= byte_size;
        }
        return ChunkHeader(chunk_index, chunk_count, total_bytes);
    }
} // sdrcomms