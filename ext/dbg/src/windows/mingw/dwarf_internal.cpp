// Copyright Edd Dawson 2012
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "dwarf_internal.hpp"
#include "memstream.hpp"

namespace dbg 
{
    attrib_spec::attrib_spec(memstream &s) :
        name(0),
        form(0) 
    {
        const uint64_t name64 = s.uleb();
        if (name64 > DW_AT_hi_user)
            throw dwarf_error("unknown DW_AT in .debug_abbrev attribute specification");

        const uint64_t form64 = s.uleb();
        if (form64 > 0x20)
            throw dwarf_error("unknown DW_FORM in .debug_abbrev attribute specification");

        name = name64 & 0xFFFF;
        form = form64 & 0xFF;
    }

    abbrev_decl::abbrev_decl(memstream &s, std::vector<attrib_spec> &specs) :
        abbrev_code(s.uleb()),
        tag(0),
        children(false),
        specs(&specs),
        first_spec(specs.size()),
        num_specs(0)
    {
        if (abbrev_code)
        {
            const uint64_t tag64 = s.uleb();
            if (tag64 > DW_TAG_hi_user)
                throw dwarf_error("unknown DW_TAG in .debug_abbrev abbreviations declaration");

            tag = tag64 & 0xFFFF;

            children = (s.u8() == DW_CHILDREN_yes);

            while (true)
            {
                attrib_spec spec(s);
                if (spec.name != 0 || spec.form != 0) specs.push_back(spec);
                else break;
            }

            num_specs = specs.size() - first_spec;
        }
    }

    std::size_t abbrev_decl::num_attribs() const
    {
        return num_specs;
    }

    const attrib_spec &abbrev_decl::attrib(std::size_t i) const
    {
        return specs->at(first_spec + i);
    }

    abbrev_table::abbrev_table(memstream &s, std::vector<attrib_spec> &specs, std::vector<abbrev_decl> &decls) :
        decls(&decls),
        first_decl(decls.size()),
        num_decls(0)
    {
        while (true)
        {
            abbrev_decl decl(s, specs);
            if (decl.abbrev_code != 0) decls.push_back(decl);
            else break;
        }

        num_decls = decls.size() - first_decl;
    }

    const abbrev_decl &abbrev_table::decl(uint64_t abbrev_code) const
    {
        for (std::size_t i = first_decl, n = first_decl + num_decls; i != n; ++i)
        {
            const abbrev_decl &decl = (*decls)[i];
            if (decl.abbrev_code == abbrev_code)
                return decl;
        }

        throw dwarf_error("invalid abbreviation code");
    }

    void debug_abbrev::load(memstream &s)
    {
        while (s.remaining())
        {
            const uint64_t offset = s.offset();
            abbrev_table tbl(s, specs, decls);

            if (tbl.num_decls != 0)
                offset2table.insert(std::make_pair(offset, tbl));
        }
    }

    const abbrev_table &debug_abbrev::table(uint64_t offset) const
    {
        std::map<uint64_t, abbrev_table>::const_iterator f = offset2table.find(offset);
        if (f == offset2table.end())
            throw dwarf_error("bad offset in to .debug_abbrev");

        return f->second;
    }

    debug_str::debug_str() : 
        base(""),
        size(1)
    {
    }

    debug_str::debug_str(const char *base, uint64_t size) :
        base(base),
        size(size)
    {
    }

    const char *debug_str::lookup(uint64_t offset) const
    {
        if (offset >= size)
            throw dwarf_error("bad offset in to .debug_str");

        return base + offset;
    }


    comp_unit::comp_unit(memstream &info, const debug_abbrev &abbrev) :
        debug_info_offset(info.offset()),
        decls(0),
        dies(0,0)
    {
        bool dwarf64 = false;
        uint64_t cu_len = info.u32();

        if (cu_len == 0xFFFFffff)
        {
            cu_len = info.u64();
            dwarf64 = true;
        }
        if (cu_len == 0) 
            return;

        offset = uint_reader(dwarf64 ? 8 : 4);

        info.skip(2); // DWARF version

        decls = &abbrev.table(offset(info));
        address = uint_reader(info.u8());

        if (cu_len < 2 + offset.byte_size + 1)
            throw dwarf_error("bad unit_length in Compilation Unit Header");

        //dies = memstream(info, info.offset(), cu_len - 2 - offset.byte_size - 1);
        dies = memstream(info, debug_info_offset, cu_len + (dwarf64 ? 12 : 4));
        dies.skip(info.offset() - debug_info_offset);
    }


    void skip_value(uint8_t form, comp_unit &cu)
    {
        memstream &s = cu.dies;

        switch (form)
        {
            case DW_FORM_addr: cu.address.skip(s); break;

            case DW_FORM_block1: s.skip(s.u8());  break;
            case DW_FORM_block2: s.skip(s.u16()); break;
            case DW_FORM_block4: s.skip(s.u32()); break;
            case DW_FORM_block:  s.skip(s.uleb()); break;

            case DW_FORM_data1: s.skip(1); break;
            case DW_FORM_data2: s.skip(2); break;
            case DW_FORM_data4: s.skip(4); break;
            case DW_FORM_data8: s.skip(8); break;
            case DW_FORM_sdata: while (s.u8() & 0x80u) { } break;
            case DW_FORM_udata: while (s.u8() & 0x80u) { } break;

            case DW_FORM_exprloc: s.skip(s.uleb()); break;

            case DW_FORM_flag: s.skip(1); break;
            case DW_FORM_flag_present: break;

            case DW_FORM_sec_offset: cu.offset.skip(s); break;

            case DW_FORM_ref1: s.skip(1); break;
            case DW_FORM_ref2: s.skip(2); break;
            case DW_FORM_ref4: s.skip(4); break;
            case DW_FORM_ref8: s.skip(8); break;
            case DW_FORM_ref_udata: while (s.u8() & 0x80u) { } break;
            case DW_FORM_ref_addr: cu.offset.skip(s); break;
            case DW_FORM_ref_sig8: s.skip(8); break;

            case DW_FORM_string: while (s.u8() != '\0') { } break;
            case DW_FORM_strp: s.skip(cu.offset.byte_size); break;

            default:
                throw dwarf_error("unknown DW_FORM in DWARF");
        }
    }

    const char *read_str_value(uint8_t form, comp_unit &cu, const debug_str &strtab)
    {
        memstream &s = cu.dies;

        if (form == DW_FORM_string)
        {
            const char *ret = reinterpret_cast<const char *>(s.base() + s.offset());
            while (s.u8() != '\0') { } // skip over the string
            return ret;
        }
        else if (form == DW_FORM_strp)
        {
            return strtab.lookup(cu.offset(s));
        }

        throw dwarf_error("expected DW_FORM_string or DW_FORM_strp");
    }

    die_listener::~die_listener() { }
    void die_listener::on_begin_die(uint16_t) { }
    void die_listener::on_end_die(uint16_t) { }

    void die_listener::on_value(uint16_t, uint8_t dw_form, comp_unit &cu)
    {
        skip_value(dw_form, cu);
    }

    void walk_debug_info_entry(comp_unit &cu, die_listener &listener)
    {
        const uint64_t abbrev_code = cu.dies.uleb();
        if (abbrev_code == 0)
        {
            // Null entry ends run of DIE siblings (DWARF 4, 2.3)
            //listener.on_tree_depth_decrease();
            return;
        }

        const abbrev_decl &decl = cu.decls->decl(abbrev_code);
        listener.on_begin_die(decl.tag);

        for (std::size_t a = 0, na = decl.num_attribs(); a != na; ++a)
        {
            const attrib_spec &attrib = decl.attrib(a);

            uint8_t form = attrib.form;
            while (form == DW_FORM_indirect) 
                form = cu.dies.uleb();

            listener.on_value(attrib.name, form, cu);
        }

        listener.on_end_die(decl.tag);

        //if (decl.children)
        //    listener.on_tree_depth_increase();
    }


    debug_info_listener::~debug_info_listener() { }
    void debug_info_listener::on_begin_comp_unit(const comp_unit &) { }
    void debug_info_listener::on_end_comp_unit(const comp_unit &) { }

    void walk_debug_info(memstream debug_info, const debug_abbrev &abbrev, debug_info_listener &listener)
    {
        while (debug_info.remaining())
        {
            comp_unit cu(debug_info, abbrev);

            listener.on_begin_comp_unit(cu);

            while (cu.dies.remaining())
                walk_debug_info_entry(cu, listener);

            listener.on_end_comp_unit(cu);

            debug_info.go(cu.debug_info_offset + cu.dies.total());
        }
    }

} // dbg
