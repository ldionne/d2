// Copyright Edd Dawson 2012
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "pe.hpp"
#include "file.hpp"

#include <vector>
#include <cstring>

namespace dbg 
{
    namespace
    {
        void read_string_table(file &pe, uint64_t tabpos, std::vector<char> &strtab)
        {
            const uint64_t old_pos = pe.offset();

            pe.go(tabpos);

            const uint32_t tab_size = pe.u32(); // includes this size field
            if (tab_size < 4)
                throw file_error("invalid size in COFF string table");
            
            std::vector<char> temp(tab_size + 1, '\0'); // extra '\0' on the end just in case
            pe.read(reinterpret_cast<uint8_t *>(&temp[4]), tab_size - 4);

            strtab.swap(temp);
            pe.go(old_pos);
        }

        uint32_t read_section_name(const char *num)
        {
            uint32_t ret = 0;
            while (*num != '\0')
            {
                if (*num < '0' || *num > '9') 
                    throw file_error("string -> number conversion failed");

                ret *= 10;
                ret += (*num++ - '0');
            }

            return ret;
        }

    } // anonymous

    std::size_t summarize_mingw_pe_file(file &pe, pe_sink &sink)
    {
        pe.go(0x3C);
        pe.go(pe.u32());
    
        // Check PE signature.
        if (pe.u8() != 'P' || pe.u8() != 'E' || pe.u16() != 0) 
            throw file_error("file is not a Portable Executable");


        // Read COFF header

        pe.skip(2); // machine
        const uint16_t number_of_sections = pe.u16();
        pe.skip(4); // timestamp

        const uint32_t abs_pos_symtab = pe.u32();
        const uint32_t number_of_symbols = pe.u32();

        const uint16_t size_of_optional_header = pe.u16();
        pe.skip(2); // characteristics

        if (abs_pos_symtab == 0) 
            throw file_error("no COFF symbols");


        // Read 'optional' header

        const uint64_t abs_pos_optional_header = pe.offset();

        // Standard fields
        const uint16_t magic = pe.u16();
        if (magic != 0x10b && magic != 0x20b) 
            throw file_error("magic number in optional header not recognized");

        const bool pe_plus = (magic == 0x20b);

        pe.skip(22);
        if (!pe_plus) pe.skip(4);

        // Windows specific fields
        sink.on_preferred_address(pe_plus ? pe.u64() : pe.u32()); // ImageBase

        const uint32_t section_alignment = pe.u32();
        pe.skip(20);
        const uint32_t size_of_image = pe.u32();

        if (pe_plus) pe.skip(48);
        else pe.skip(32);

        const uint32_t number_of_rva_and_sizes = pe.u32();

        // Data directories
        pe.skip(number_of_rva_and_sizes * 8);

        if (pe.offset() - abs_pos_optional_header != size_of_optional_header)
            throw file_error("size of optional header did not match expectation");


        // PE/COFF spec says that section names in images should be no longer
        // than 8 bytes, but recommended DWARF section names are longer so
        // MinGW has to ignore this and use string table.
        std::vector<char> strtab;
        read_string_table(pe, abs_pos_symtab + 18 * number_of_symbols, strtab);

        // Read sections table
        for (uint16_t s = 0; s != number_of_sections; ++s)
        {
            const char *section_name = "";

            char buff[9];
            pe.read(reinterpret_cast<uint8_t *>(buff), 8);
            buff[8] = '\0';

            if (buff[0] == '/') section_name = &strtab.at(read_section_name(buff + 1));
            else section_name = buff;

            const std::size_t virtual_size = pe.u32();
            const std::size_t virtual_address = pe.u32();
            const std::size_t size_of_raw_data = pe.u32();
            const std::size_t pointer_to_raw_data = pe.u32();

            const std::size_t size_of_section = std::min(size_of_raw_data, virtual_size);

            pe.skip(16);

            if (virtual_address % section_alignment == 0)
                sink.on_section(section_name, virtual_address, pointer_to_raw_data, size_of_section);
        }

        return size_of_image;
    }

} // dbg
