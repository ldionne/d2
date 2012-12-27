// Copyright Edd Dawson 2012
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef PE_HPP_0129_29062012
#define PE_HPP_0129_29062012

#include <cstddef>

namespace dbg 
{
    class file;

    struct pe_sink
    {
        virtual ~pe_sink() { }

        virtual void on_preferred_address(std::size_t addr) = 0;

        // Implementations should take a copy of name if needed, as the storage only
        // exists for the duration of the on_section() call.
        virtual void on_section(const char *name, 
                                std::size_t image_offset,
                                std::size_t file_offset,
                                std::size_t length) = 0;
    };

    // Find the sections in a windows Portable Executable and expose them to the
    // given sink. Returns the size of the image once loaded in to memory by the
    // Windows loader.
    std::size_t summarize_mingw_pe_file(file &pe, pe_sink &sink);

} // dbg

#endif // PE_HPP_0129_29062012
