// Copyright Edd Dawson 2012
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef DWARF_INTERNAL_HPP_0036_30062012
#define DWARF_INTERNAL_HPP_0036_30062012

#include "memstream.hpp"

#include <cstddef>
#include <vector>
#include <map>

#include <stdint.h>

namespace dbg 
{
    const uint8_t DW_CHILDREN_no  = 0x00;
    const uint8_t DW_CHILDREN_yes = 0x01;

    const uint8_t DW_FORM_addr = 0x01;
    const uint8_t DW_FORM_block2 = 0x03;
    const uint8_t DW_FORM_block4 = 0x04;
    const uint8_t DW_FORM_data2 = 0x05;
    const uint8_t DW_FORM_data4 = 0x06;
    const uint8_t DW_FORM_data8 = 0x07;
    const uint8_t DW_FORM_string = 0x08;
    const uint8_t DW_FORM_block = 0x09;
    const uint8_t DW_FORM_block1 = 0x0a;
    const uint8_t DW_FORM_data1 = 0x0b;
    const uint8_t DW_FORM_flag = 0x0c;
    const uint8_t DW_FORM_sdata = 0x0d;
    const uint8_t DW_FORM_strp = 0x0e;
    const uint8_t DW_FORM_udata = 0x0f;
    const uint8_t DW_FORM_ref_addr = 0x10;
    const uint8_t DW_FORM_ref1 = 0x11;
    const uint8_t DW_FORM_ref2 = 0x12;
    const uint8_t DW_FORM_ref4 = 0x13;
    const uint8_t DW_FORM_ref8 = 0x14;
    const uint8_t DW_FORM_ref_udata = 0x15;
    const uint8_t DW_FORM_indirect = 0x16;
    const uint8_t DW_FORM_sec_offset = 0x17;
    const uint8_t DW_FORM_exprloc = 0x18;
    const uint8_t DW_FORM_flag_present = 0x19;
    const uint8_t DW_FORM_ref_sig8 = 0x20;


    // Incomplete, only what's needed:
    const uint8_t DW_TAG_class_type = 0x02;
    const uint8_t DW_TAG_entry_point = 0x03;
    const uint8_t DW_TAG_imported_declaration = 0x08;
    const uint8_t DW_TAG_lexical_block = 0x0b;
    const uint8_t DW_TAG_structure_type = 0x13;
    const uint8_t DW_TAG_union_type = 0x17;
    const uint8_t DW_TAG_inlined_subroutine = 0x1d;
    const uint8_t DW_TAG_subprogram = 0x2e;
    const uint8_t DW_TAG_namespace = 0x39;
    const uint16_t DW_TAG_hi_user = 0xffff;


    // Incomplete, only what's needed:
    const uint8_t DW_AT_name = 0x03;
    const uint8_t DW_AT_low_pc = 0x11;
    const uint8_t DW_AT_import = 0x18;
    const uint8_t DW_AT_abstract_origin = 0x31;
    const uint8_t DW_AT_specification = 0x47;
    const uint8_t DW_AT_entry_pc = 0x52;
    const uint8_t DW_AT_extension = 0x54;
    const uint8_t DW_AT_description = 0x5a;
    const uint8_t DW_AT_linkage_name = 0x6e;
    const uint16_t DW_AT_MIPS_linkage_name = 0x2007;
    const uint16_t DW_AT_hi_user = 0x3fff;


    class dwarf_error : public std::exception
    {
        public:
            explicit dwarf_error(const char *err) : err(err) { }
            ~dwarf_error() throw() { }

            const char *what() const throw() { return err; }

        private:
            const char *err;
    };

    struct attrib_spec
    {
        attrib_spec(memstream &s);

        uint16_t name;
        uint8_t form;
    };

    struct abbrev_decl
    {
        abbrev_decl(memstream &s, std::vector<attrib_spec> &specs);

        std::size_t num_attribs() const;
        const attrib_spec &attrib(std::size_t i) const;

        uint64_t abbrev_code;
        uint16_t tag;
        bool children;

        const std::vector<attrib_spec> *specs;
        std::size_t first_spec;
        std::size_t num_specs;
    };

    struct abbrev_table
    {
        abbrev_table(memstream &s, std::vector<attrib_spec> &specs, std::vector<abbrev_decl> &decls);

        const abbrev_decl &decl(uint64_t abbrev_code) const;

        const std::vector<abbrev_decl> *decls;
        std::size_t first_decl;
        std::size_t num_decls;
    };

    // Loads/contains the abbreviations table from the DWARF .debug_abbrev section.
    class debug_abbrev
    {
        public:
            void load(memstream &s);
            const abbrev_table &table(uint64_t offset) const;

        private:
            std::vector<attrib_spec> specs;
            std::vector<abbrev_decl> decls;
            std::map<uint64_t, abbrev_table> offset2table;
    };

    // Loads/contains the string table from the DWARF .debug_str section.
    class debug_str
    {
        public:
            debug_str();
            debug_str(const char *base, uint64_t size);

            const char *lookup(uint64_t offset) const;

        private:
            const char *base;
            uint64_t size;
    };

    // Used to read a compilation unit from the DWARF .debug_info section.
    struct comp_unit
    {
        // Reads the compilation unit header from the current position in
        // the given memstream and prepares the other data members so that
        // the contained DIEs can be processed.
        comp_unit(memstream &info, const debug_abbrev &abbrev);

        uint64_t debug_info_offset;

        uint_reader offset;
        uint_reader address;

        const abbrev_table *decls;
        memstream dies;
    };

    // Reads a string from the specified compilation unit. It can be any of the DWARF
    // string forms. A pointer to the string is returned and the read cursor of cu.dies
    // is advanced to the next piece of data.
    const char *read_str_value(uint8_t form, comp_unit &cu, const debug_str &strtab);

    // Reads the next value from the compilation unit and throws it away.
    // The read cursor of cu.dies will then point to the next piece of data.
    void skip_value(uint8_t form, comp_unit &cu);

    struct die_listener
    {
        virtual ~die_listener();

        virtual void on_begin_die(uint16_t dw_tag); // does nothing by default
        virtual void on_end_die(uint16_t dw_tag); // does nothing by default

        // derived implementation is expected to read value from cu.dies. Skips value by default
        virtual void on_value(uint16_t dw_at, uint8_t dw_form, comp_unit &cu);
    };

    // Reads the next DIE from the specified compilation unit. The data are emitted
    // through the specified die_listener.
    void walk_debug_info_entry(comp_unit &cu, die_listener &listener);

    struct debug_info_listener : die_listener
    {
        virtual ~debug_info_listener();
        
        virtual void on_begin_comp_unit(const comp_unit &cu); // does nothing by deafult
        virtual void on_end_comp_unit(const comp_unit &cu); // does nothing by deafult

    };

    // Walks the entire .debug_info data in the specified memstream. The data are
    // emitted through the spevified debug_info_listener.
    void walk_debug_info(memstream debug_info, const debug_abbrev &abbrev, debug_info_listener &listener); 

} // dbg

#endif // DWARF_INTERNAL_HPP_0036_30062012
