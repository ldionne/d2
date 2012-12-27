// Copyright Edd Dawson 2012
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "memstream.hpp"
#include "memcpy_cast.hpp"

#include <algorithm>
#include <cstring>

#include <windows.h>

namespace dbg 
{
    memstream::memstream() :
        begin(0),
        next(0),
        left(0)
    {
    }

    memstream::memstream(const uint8_t *begin, std::size_t size) :
        begin(begin),
        next(begin),
        left(size)
    {
    }

    memstream::memstream(const memstream &parent, std::size_t start, std::size_t size) :
        begin(parent.base() + start),
        next(parent.base() + start),
        left(size)
    {
        if (start + size > parent.total())
            throw stream_error("bad bounds specified in creation of child memory stream");
    }

    const uint8_t *memstream::base() const
    {
        return begin;
    }

    std::size_t memstream::total() const
    {
        return offset() + left;
    }

    std::size_t memstream::offset() const
    {
        return next - begin;
    }

    std::size_t memstream::remaining() const
    {
        return left;
    }

    void memstream::go(std::size_t position)
    {
        if (position > offset() + left)
            throw stream_error("bad stream offset");

        const uint8_t *end = next + left;
        next = begin + position;
        left = end - next;
    }

    void memstream::skip(std::ptrdiff_t delta)
    {
        if (delta < 0 && static_cast<std::size_t>(-delta) > offset())
            throw stream_error("bad stream delta");
        else if (delta > 0 && static_cast<std::size_t>(delta) > left)
            throw stream_error("bad stream delta");

        left -= delta;
        next += delta;
    }

    void memstream::read(uint8_t *out, std::size_t n)
    {
        if (n > left)
            throw stream_error("stream exhausted");

        std::memcpy(out, next, n);

        left -= n;
        next += n;
    }

    uint64_t memstream::u64()
    {
        uint64_t ret = 0;
        read(reinterpret_cast<uint8_t *>(&ret), sizeof ret);
        return ret;
    }

    uint32_t memstream::u32()
    {
        uint32_t ret = 0;
        read(reinterpret_cast<uint8_t *>(&ret), sizeof ret);
        return ret;
    }

    uint16_t memstream::u16()
    {
        uint16_t ret = 0;
        read(reinterpret_cast<uint8_t *>(&ret), sizeof ret);
        return ret;
    }

    uint8_t memstream::u8()
    {
        if (left == 0)
            throw stream_error("stream exhausted");

        left--;
        return *next++;
    }

    uint64_t memstream::sleb()
    {
        uint64_t ret = 0;

        uint8_t b = 0;
        unsigned shift = 0;

        do
        {
            if (shift >= 64)
                throw stream_error("ULEB128 overflowed 64bit value");

            b = u8();
            ret |= (uint64_t(b & 0x7F) << shift);
            shift += 7;
        }
        while ((b & 0x80) != 0);

        if ((shift < 64) && (b & 0x80) != 0)
            ret |= ~((uint64_t(1) << shift) - uint64_t(1));
        
        return memcpy_cast<int64_t>(ret);
    }

    uint64_t memstream::uleb()
    {
        uint64_t ret = 0;

        uint8_t b = 0;
        unsigned shift = 0;
        do
        {
            if (shift >= 64)
                throw stream_error("ULEB128 overflowed 64bit value");

            b = u8();
            ret |= (uint64_t(b & 0x7F) << shift);
            shift += 7;
        }
        while ((b & 0x80) != 0);

        return ret;
    }

    void memstream::swap(memstream &other)
    {
        std::swap(begin, other.begin);
        std::swap(next,  other.next);
        std::swap(left,  other.left);
    }

    uint_reader::uint_reader(unsigned byte_size) : 
        byte_size(byte_size) 
    {
        if (byte_size == 0 || byte_size > 8)
            throw stream_error("unsupported integer size");
    }

    uint64_t uint_reader::operator() (memstream &s) const
    {
        uint64_t ret = 0;
        s.read(reinterpret_cast<uint8_t *>(&ret), byte_size);
        return ret;
    }

    void uint_reader::skip(memstream &s) const
    {
        s.skip(byte_size);
    }


    namespace
    {
        bool readable(const MEMORY_BASIC_INFORMATION &info)
        {
            const unsigned readable_bits =
                PAGE_EXECUTE_READ |
                PAGE_EXECUTE_READWRITE |
                PAGE_EXECUTE_WRITECOPY |
                PAGE_READONLY |
                PAGE_READWRITE |
                PAGE_WRITECOPY;

            return 
                info.State == MEM_COMMIT && 
                (info.Protect & readable_bits) != 0 &&
                (info.Protect & (PAGE_GUARD | PAGE_NOACCESS)) == 0;
        }

        bool in_image(const MEMORY_BASIC_INFORMATION &info)
        {
            return (info.State & MEM_FREE) == 0 && (info.Type & MEM_IMAGE) != 0;
        }

        bool query_region(const void *start, std::size_t length, bool predicate(const MEMORY_BASIC_INFORMATION &))
        {
            if (length == 0)
                return false;

            MEMORY_BASIC_INFORMATION info;
            std::memset(&info, 0, sizeof info);

            if (VirtualQuery(start, &info, sizeof info) != sizeof info)
                return false;

            const uintptr_t addr = memcpy_cast<uintptr_t>(start);

            const uintptr_t begin = memcpy_cast<uintptr_t>(info.BaseAddress);
            const uintptr_t end = begin + info.RegionSize;

            if (addr < begin || addr + length > end)
                return false;

            return predicate(info);
        }

    } // anonymous

    bool region_is_readable(const void *start, std::size_t length)
    {
        return query_region(start, length, readable);
    }

    bool region_is_readable(memstream &s)
    {
        return region_is_readable(s.base(), s.total());
    }

    bool region_in_image(const void *start, std::size_t length)
    {
        return query_region(start, length, in_image);
    }

    bool region_in_image(memstream &s)
    {
        return region_in_image(s.base(), s.total());
    }

} // dbg
