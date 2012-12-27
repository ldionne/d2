// Copyright Edd Dawson 2012
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "die_lookup.hpp"
#include "dwarf_internal.hpp"

#include <algorithm>
#include <cassert>
#include <functional>

namespace dbg 
{
    namespace
    {
        struct offset_collector : debug_info_listener
        {
            void on_begin_comp_unit(const comp_unit &cu) { cu_offsets.push_back(cu.debug_info_offset); }
            std::vector<uint64_t> cu_offsets;
        };

    } // anonymous

    die_lookup::die_lookup(const memstream &debug_info, const debug_abbrev &abbrev): 
        debug_info(debug_info),
        abbrev(abbrev)
    {
        offset_collector collector;
        walk_debug_info(debug_info, abbrev, collector);

        cu_offsets.swap(collector.cu_offsets);
    }

    die_lookup::~die_lookup()
    {
    }

    comp_unit die_lookup::get_from_ref_value(uint64_t ref_dw_form, comp_unit &cu) const
    {
        comp_unit ret(cu);

        // Set referenced_cu to the compilation unit containing the target DIE and
        // position its dies member at that DIE.
        switch (ref_dw_form)
        {
            case DW_FORM_ref1: ret.dies.go(cu.dies.u8()); break;
            case DW_FORM_ref2: ret.dies.go(cu.dies.u16()); break;
            case DW_FORM_ref4: ret.dies.go(cu.dies.u32()); break;
            case DW_FORM_ref8: ret.dies.go(cu.dies.u64()); break;
            case DW_FORM_ref_udata: ret.dies.go(cu.dies.uleb()); break;
            case DW_FORM_ref_addr: ret = get_from_debug_info(cu.offset(cu.dies)); break;
            case DW_FORM_ref_sig8: throw dwarf_error("DW_FORM_ref_sig8 not supported"); break;
            default: throw dwarf_error("invalid form for DWARF reference"); break;
        }

        return ret;
    }

    comp_unit die_lookup::get_from_debug_info(uint64_t global_die_offset) const
    {
        std::vector<uint64_t>::const_iterator f = 
            std::lower_bound(cu_offsets.begin(), cu_offsets.end(), global_die_offset);

        if (f == cu_offsets.end() || global_die_offset < *f)
            if (f != cu_offsets.begin()) 
                --f;

        if (f != cu_offsets.end())
        {
            memstream di_temp(debug_info);
            di_temp.go(*f);

            comp_unit cu(di_temp, abbrev);
            
            if (global_die_offset >= cu.debug_info_offset + cu.dies.offset())
            { 
                cu.dies.go(global_die_offset - cu.debug_info_offset);
                return cu;
            }
        }

        throw dwarf_error("invalid DW_FORM_ref_addr destination");
    }

} // dbg
