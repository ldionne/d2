// Copyright Edd Dawson 2012
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef DIE_LOOKUP_HPP_1428_01072012
#define DIE_LOOKUP_HPP_1428_01072012

#include "memstream.hpp"

#include <vector>
#include <stdint.h>

namespace dbg 
{
    class debug_abbrev;
    struct comp_unit;

    // Used as part of the DWARF-skimming code.
    // Locates DIEs in .debug_info.
    class die_lookup
    {
        public:
            // Construct a die_lookup from the DWARF .debug_info data and the
            // abbreviations table from .debug_abbrev.
            // All compilation units are found and their offsets stored for
            // subsequent DIE lookups.
            die_lookup(const memstream &debug_info, const debug_abbrev &abbrev);
            ~die_lookup();

            // Returns a comp_unit whose dies member will be at the DIE identified by
            // reading the next form attribute from cu.
            comp_unit get_from_ref_value(uint64_t ref_dw_form, comp_unit &cu) const;

            // Returns a comp_unit whose dies member will be at the DIE identified by
            // the global byte-offset in to .debug_info.
            comp_unit get_from_debug_info(uint64_t global_die_offset) const;

        private:
            die_lookup(const die_lookup &);
            die_lookup &operator= (const die_lookup &);

        private:
            const memstream debug_info;
            const debug_abbrev &abbrev;
            std::vector<uint64_t> cu_offsets;
    };

} // dbg

#endif // DIE_LOOKUP_HPP_1428_01072012
