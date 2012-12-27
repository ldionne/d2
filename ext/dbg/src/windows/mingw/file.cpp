// Copyright Edd Dawson 2012
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "file.hpp"

#include <windows.h>

namespace dbg 
{
    file::file(const wchar_t *filename) :
        h(CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0)),
        sz(0),
        cursor(0)
    {
        if (!h || h == INVALID_HANDLE_VALUE)
            throw file_error("failed to open file");

        LARGE_INTEGER file_size;
        if (!GetFileSizeEx(h, &file_size))
        {
            CloseHandle(h);
            throw file_error("failed to get file size");
        }
        sz = file_size.QuadPart;
    }

    file::~file()
    {
        CloseHandle(h);
    }


    uint64_t file::size() const
    {
        return sz;
    }

    void file::go(uint64_t pos)
    {
        if (pos == cursor)
            return;

        if (pos > sz)
            throw file_error("bad stream cursor position specified");

        LARGE_INTEGER file_pos;
        file_pos.QuadPart = pos;

        LARGE_INTEGER new_cursor;

        if (!SetFilePointerEx(h, file_pos, &new_cursor, FILE_BEGIN))
            throw file_error("failed to move stream cursor");
    }

    void file::skip(int64_t delta)
    {
        if (delta == 0)
            return;

        if ((delta < 0 && static_cast<uint64_t>(-delta) > cursor) ||
            (delta > 0 && static_cast<uint64_t>(delta) + cursor > sz))
        {
            throw file_error("bad stream cursor position specified");
        }

        LARGE_INTEGER file_delta;
        file_delta.QuadPart = delta;

        LARGE_INTEGER new_cursor;

        if (!SetFilePointerEx(h, file_delta, &new_cursor, FILE_CURRENT))
            throw file_error("failed to move stream cursor");

        cursor = new_cursor.QuadPart;
    }

    uint64_t file::offset() const
    {
        return cursor;
    }

    uint64_t file::u64()
    {
        uint64_t ret = 0;
        DWORD num_read = 0;
        if (!ReadFile(h, &ret, sizeof(ret), &num_read, 0) || num_read != sizeof(ret))
            throw file_error("failed to read from file");

        cursor += sizeof(ret);
        return ret;
    }

    uint32_t file::u32()
    {
        uint32_t ret = 0;
        DWORD num_read = 0;
        if (!ReadFile(h, &ret, sizeof(ret), &num_read, 0) || num_read != sizeof(ret))
            throw file_error("failed to read from file");

        cursor += sizeof(ret);
        return ret;
    }

    uint16_t file::u16()
    {
        uint16_t ret = 0;
        DWORD num_read = 0;
        if (!ReadFile(h, &ret, sizeof(ret), &num_read, 0) || num_read != sizeof(ret))
            throw file_error("failed to read from file");

        cursor += sizeof(ret);
        return ret;
    }

    uint8_t file::u8()
    {
        uint8_t ret = 0;
        DWORD num_read = 0;
        if (!ReadFile(h, &ret, sizeof(ret), &num_read, 0) || num_read != sizeof(ret))
            throw file_error("failed to read from file");

        cursor += sizeof(ret);
        return ret;
    }

    void file::read(uint8_t *bytes, std::size_t n)
    {
        while (n != 0)
        {
            const DWORD wanted = (n > DWORD(-1)) ? DWORD(-1) : n;
            DWORD num_read = 0;

            if (!ReadFile(h, bytes, wanted, &num_read, 0) || num_read != wanted)
                throw file_error("failed to read from file");

            bytes += num_read;
            n -= num_read;
            cursor += num_read;
        }
    }

} // dbg
