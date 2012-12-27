// Copyright Edd Dawson 2012
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMSTREAM_HPP_2132_29062012
#define MEMSTREAM_HPP_2132_29062012

#include <stdint.h>
#include <cstddef>
#include <exception>

namespace dbg 
{
    class stream_error : public std::exception
    {
        public:
            explicit stream_error(const char *err) : err(err) { }
            virtual ~stream_error() throw () { }

            virtual const char *what() const throw() { return err; }

        private:
            const char *err;
    };

    // Used to read bytes and integers from a binary memory stream.
    // It assumes the memory content and host system are little endian.
    class memstream
    {
        public:
            // Create an empty (exhausted) memstream.
            memstream();

            // Create a memory stream referring to the region of bytes in the range
            // [begin, begin + size).
            memstream(const uint8_t *begin, std::size_t size);

            // Construct a memstream referring to a subset of the parent's data.
            // Throws stream_error if start + size > parent.total().
            memstream(const memstream &parent, std::size_t start, std::size_t size);

            // Returns a pointer to the beginning of the region of memory specified
            // on construction.
            const uint8_t *base() const;

            // Returns the total number of bytes in the memory region specified on
            // construction.
            std::size_t total() const;

            // Returns the position of the read cursor after the beginning of the
            // region of memory specified on construction.
            std::size_t offset() const;

            // Equivalent to total() - offset().
            std::size_t remaining() const;

            // Move the read cursor to the specified absolute position.
            // Throws stream_error if the position is not valid.
            void go(std::size_t position);

            // Move the read cursor by the speified amount, relative to its current
            // position.
            // Throws stream_error if the resulting position is not valid.
            void skip(std::ptrdiff_t delta);

            // The read functions throw stream_error if there are insufficient bytes
            // remaining in the stream. The read cursor is advanced on success.
            void read(uint8_t *out, std::size_t n);

            uint64_t u64();
            uint32_t u32();
            uint16_t u16();
            uint8_t u8();

            // Read signed/unsigned LEB128 values (see DWARF4, 7.6).
            // A stream_error is thrown if the end of the stream is encountered before
            // the LEB128 value can be fully decoded. The read cursor is advanced on
            // success.
            uint64_t sleb(); 
            uint64_t uleb();

            // Swap the guts of this memstream with those of other.
            void swap(memstream &other);

        private:
            const uint8_t *begin;
            const uint8_t *next;
            std::size_t left;
    };

    // A uint_reader can be used to read/skip an unsigned integer of the byte size 
    // specified on construction. It is always returned as a unit64_t.
    // uint_readers are typically used to read offsets in DWARF DIEs, whose sizes
    // are dependent on the pointer size of the machine.
    struct uint_reader
    {
        uint_reader() : byte_size(0) { }
        uint_reader(unsigned byte_size);
        uint64_t operator() (memstream &s) const;
        void skip(memstream &s) const;

        unsigned byte_size;
    };

    // Returns true if the specified range of bytes is readable.
    bool region_is_readable(const void *start, std::size_t length);
    bool region_is_readable(memstream &s);

    // Returns true if the specified range of bytes is part of the process
    // image created by the Windows loader.
    bool region_in_image(const void *start, std::size_t length);
    bool region_in_image(memstream &s);

} // dbg

#endif // MEMSTREAM_HPP_2132_29062012
