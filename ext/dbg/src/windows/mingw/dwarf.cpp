// Copyright Edd Dawson 2012
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "dwarf.hpp"
#include "dwarf_internal.hpp"
#include "die_lookup.hpp"
#include "pe.hpp"
#include "file.hpp"
#include "memstream.hpp"
#include "memcpy_cast.hpp"

#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <list>
#include <vector>

#include <stdint.h>

#include <cxxabi.h>

namespace dbg 
{
    namespace
    {
        struct section
        {
            section() : image_offset(0), file_offset(0), length(0) { }

            section(std::size_t image_offset, std::size_t file_offset, std::size_t length) :
                image_offset(image_offset),
                file_offset(file_offset),
                length(length) 
            {
            }

            std::size_t image_offset;
            std::size_t file_offset;
            std::size_t length;
        };

        struct dwarf_sections
        {
            dwarf_sections() : preferred_image_address(0), loaded_image_size(0) { }

            std::size_t preferred_image_address;
            std::size_t loaded_image_size;

            section debug_str;
            section debug_abbrev;
            section debug_types;
            section debug_info;
        };

        bool streq(const char *lhs, const char *rhs) 
        { 
            return std::strcmp(lhs, rhs) == 0; 
        }

        struct dwarf_section_sink : pe_sink
        {
            explicit dwarf_section_sink(dwarf_sections &sections) : sections(sections) { }
        
            virtual void on_preferred_address(std::size_t addr)
            {
                sections.preferred_image_address = addr;
            }

            virtual void on_section(const char *name, uint32_t image_offset, uint32_t file_offset, uint32_t length)
            {
                section *target =
                    streq(name, ".debug_str")    ? &sections.debug_str    :
                    streq(name, ".debug_abbrev") ? &sections.debug_abbrev :
                    streq(name, ".debug_types")  ? &sections.debug_types  :
                    streq(name, ".debug_info")   ? &sections.debug_info   :
                    0;

                if (target)
                {
                    if (target->length) // we've already seen this section!?
                        throw file_error("conflicting/duplicate DWARF sections in PE file");

                    *target = section(image_offset, file_offset, length);
                }
            }

            dwarf_sections &sections;
        };

        dwarf_sections dwarf_sections_from_mingw_pe_file(file &pe)
        {
            dwarf_sections ret;
            dwarf_section_sink sink(ret);
            ret.loaded_image_size = summarize_mingw_pe_file(pe, sink);

            return ret;
        }

        struct function_location
        {
            function_location(std::size_t address, const char *name, bool mangled) :
                address(address),
                name(name),
                mangled(mangled)
            {
            }

            std::size_t address;
            const char *name;
            bool mangled;
        };

        struct function_location_less
        {
            bool operator() (const function_location &lhs, const function_location &rhs) const
            {
                if (lhs.address < rhs.address) return true;
                if (lhs.address > rhs.address) return false;
                
                return lhs.mangled && !rhs.mangled; // mangled names considered 'less'
            }
        };

        struct function_location_address_less
        {
            bool operator() (const function_location &lhs, const function_location &rhs) const
            {
                return lhs.address < rhs.address;
            }
        };

        struct function_location_address_equal
        {
            bool operator() (const function_location &lhs, const function_location &rhs) const
            {
                return lhs.address == rhs.address;
            }
        };

        class function_details_collector : public debug_info_listener
        {
            public:
                function_details_collector(const die_lookup &lookup, const debug_str &strtab) : 
                    lookup(lookup),
                    strtab(strtab),
                    die_is_function(false),
                    recursion_depth(0),
                    die_name(0),
                    die_name_mangled(false),
                    address(0)
                {
                }

                virtual void on_begin_die(uint16_t dw_tag)
                {
                    if (recursion_depth != 0) return;

                    die_name = 0;
                    address = 0;
                    die_name_mangled = false;

                    die_is_function =
                        dw_tag == DW_TAG_subprogram ||
                        dw_tag == DW_TAG_inlined_subroutine ||
                        dw_tag == DW_TAG_entry_point;
                }

                virtual void on_end_die(uint16_t)
                {
                    if (recursion_depth != 0) return;
                    if (!die_is_function) return;
                    if (address == 0) return;

                    functions.push_back(function_location(address, die_name, die_name_mangled));
                }

                virtual void on_value(uint16_t dw_at, uint8_t dw_form, comp_unit &cu)
                {
                    if (recursion_depth == 0 && !die_is_function)
                    {
                        skip_value(dw_form, cu);
                        return;
                    }

                    if (dw_at == DW_AT_linkage_name || dw_at == DW_AT_MIPS_linkage_name)
                    {
                        die_name = read_str_value(dw_form, cu, strtab);
                        die_name_mangled = true;
                    }
                    else if (dw_at == DW_AT_name)
                    {
                        const char *temp = read_str_value(dw_form, cu, strtab);
                        if (!die_name) // linkage name takes precedence
                        {
                            die_name = temp;
                            die_name_mangled = false;
                        }
                    }
                    else if (dw_at == DW_AT_specification || dw_at == DW_AT_abstract_origin)
                    {
                        if (die_name_mangled == false && recursion_depth < 32)
                        {
                            ++recursion_depth;
                            comp_unit refcu = lookup.get_from_ref_value(dw_form, cu);
                            walk_debug_info_entry(refcu, *this);
                            --recursion_depth;
                        }
                        else
                            skip_value(dw_form, cu);
                    }
                    else if (dw_at == DW_AT_low_pc)
                    {
                        address = cu.address(cu.dies);
                    }
                    else if (dw_at == DW_AT_entry_pc)
                    {
                        if (address == 0) // DW_AT_low_pc takes precedence
                            address = cu.address(cu.dies);
                    }
                    else 
                        skip_value(dw_form, cu);
                }

                void take_found_functions(std::vector<function_location> &store)
                {
                    store.swap(functions);
                    functions.clear();
                }

            private:
                const die_lookup &lookup;
                const debug_str strtab;

                bool die_is_function;
                unsigned recursion_depth;

                const char *die_name;
                bool die_name_mangled;
                uint64_t address;

                std::vector<function_location> functions;
        };

        class mem_blocks
        {
            public:
                mem_blocks() { }

                uint8_t *allocate(std::size_t section_size)
                {
                    if (section_size == 0)
                        return 0;

                    std::vector<uint8_t> temp;
                    mem.push_back(temp);

                    temp.resize(section_size);
                    mem.back().swap(temp);

                    return &mem.back().front();
                }

                void release(const uint8_t *block)
                {
                    for (std::list<std::vector<uint8_t> >::iterator it = mem.begin(), e = mem.end(); it != e; ++it)
                    {
                        if (&it->front() == block)
                        {
                            mem.erase(it);
                            return;
                        }
                    }
                }

            private:
                mem_blocks(const mem_blocks &);
                mem_blocks &operator= (const mem_blocks &);

            private:
                std::list<std::vector<uint8_t> > mem;
        };

    } // anonymous

    class dwarf::impl
    {
        public:
            impl(const void *module_image, file &original_pe);
            ~impl();

            const char *function_spanning(const void *program_counter);

        private:
            void find_functions(memstream &info);
            void get_section_stream(const section &sec, file &original_pe, const memstream &module, memstream &stream);

        private:
            mem_blocks blocks;

            debug_str str;
            debug_abbrev abbrev;

            std::size_t offset_from_preferred;
            std::vector<function_location> functions;

            char *demangle_buffer;
            std::size_t demangle_buffer_size;
    };

    dwarf::impl::impl(const void *module_image, file &original_pe) :
        str("", 1),
        offset_from_preferred(0),
        demangle_buffer(0),
        demangle_buffer_size(0)
    {
        const dwarf_sections sections = dwarf_sections_from_mingw_pe_file(original_pe);

        // The module may not have been loaded at its preferred address.
        // We therefore need to make sure we apply an offset to the address of
        // functions for which symbols are requested.
        const std::size_t image_base = memcpy_cast<std::size_t>(module_image);
        offset_from_preferred = image_base - sections.preferred_image_address; // relying on unsigned wrapping

        const memstream mod(static_cast<const uint8_t *>(module_image), sections.loaded_image_size);

        // Read .debug_str
        if (sections.debug_str.length)
        {
            memstream ds;
            get_section_stream(sections.debug_str, original_pe, mod, ds);

            ds.go(ds.total() - 1);
            if (ds.u8() != '\0')
                throw dwarf_error(".debug_str does not end in nul");

            str = debug_str(reinterpret_cast<const char *>(ds.base()), ds.total());
        }

        // Read .debug_abbrev
        if (sections.debug_abbrev.length)
        {
            memstream da;
            get_section_stream(sections.debug_abbrev, original_pe, mod, da);
            abbrev.load(da);

            // .debug_abbrev memory not needed anymore, so release it if we loaded it from disk.
            blocks.release(da.base()); 
        }

        // Find the functions and their addresses in .debug_info
        if (sections.debug_info.length)
        {
            memstream di;
            get_section_stream(sections.debug_info, original_pe, mod, di);
            find_functions(di);
        }
    }

    dwarf::impl::~impl()
    {
        std::free(demangle_buffer);
    }

    const char *dwarf::impl::function_spanning(const void *program_counter)
    {
        const std::size_t pc = memcpy_cast<std::size_t>(program_counter) - offset_from_preferred;
        const function_location needle(pc, 0, false);

        std::vector<function_location>::const_iterator f =
            std::lower_bound(functions.begin(), functions.end(), needle, function_location_address_less());

        if (f == functions.end() || pc < f->address)
        {
            if (f == functions.begin()) return 0;
            --f;
        }

        if (f->mangled == false)
            return f->name;

        int status = 0;
        std::size_t len = 0;

        char *demangled = abi::__cxa_demangle(f->name, demangle_buffer, &len, &status);

        if (demangled)
        {
            demangle_buffer = demangled; // reuse the space allocated by __cxa_demangle
            demangle_buffer_size = len;
        }

        return demangled;
    }

    void dwarf::impl::find_functions(memstream &info)
    {
        const die_lookup lookup(info, abbrev);

        function_details_collector collector(lookup, str);
        walk_debug_info(info, abbrev, collector);

        collector.take_found_functions(functions);

        // Remove non-uniquely-addressed functions, keeping mangled names in preference.
        std::sort(functions.begin(), functions.end(), function_location_less());
        functions.erase(
            std::unique(functions.begin(), functions.end(), function_location_address_equal()),
            functions.end()
        );
    }

    void dwarf::impl::get_section_stream(const section &sec, file &original_pe,
                                         const memstream &module, memstream &stream)
    {
        memstream ret(module, sec.image_offset, sec.length);

        if (!region_is_readable(ret) || !region_in_image(ret))
        {
            // Some older versions of GCC set IMAGE_SCN_MEM_DISCARDABLE in the PE section header
            // for DWARF sections allowing the loader to discard it. Load it again here.
            
            uint8_t *mem = blocks.allocate(sec.length);
            original_pe.go(sec.file_offset);
            original_pe.read(mem, sec.length);
           
            memstream reloaded(mem, sec.length);
            ret.swap(reloaded);
        }

        stream.swap(ret);
    }



    dwarf::dwarf() :
        p(0)
    {
    }

    dwarf::dwarf(const void *module_image, file &original_pe) :
        p(new impl(module_image, original_pe))
    {
    }

    dwarf::~dwarf()
    {
        delete p;
    }

    bool dwarf::empty() const
    {
        return p == 0;
    }

    void dwarf::swap(dwarf &other)
    {
        std::swap(p, other.p);
    }

    const char *dwarf::function_spanning(const void *program_counter) const
    {
        return p ? p->function_spanning(program_counter) : 0;
    }

} // dbg
