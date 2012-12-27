// Copyright Edd Dawson 2012
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "dbg/symbols.hpp"

#include "dwarf.hpp"
#include "file.hpp"
#include "memstream.hpp"
#include "ms_symdb.hpp"

#include <cassert>
#include <cstring>
#include <new>

#include <windows.h>

namespace dbg 
{
    namespace
    {
        class module_node
        {
            public:
                module_node(HMODULE mod, const module_node *next) :
                    mod(mod),
                    own_ref(false),
                    next(next)
                {
                    wchar_t utf16_name[32 * 1024] = L"";

                    const DWORD buffsize = sizeof utf16_name / sizeof *utf16_name;
                    DWORD len = GetModuleFileNameW(mod, utf16_name, buffsize);

                    if (len == buffsize)
                        utf16_name[--len] = L'\0';

                    // Store it as UTF-8.
                    if (len == 0 || 
                        WideCharToMultiByte(CP_UTF8, 0, utf16_name, -1, utf8_name, sizeof utf8_name, 0, 0) == 0)
                    {
                        utf8_name[0] = '\0';
                    }

                    // Load debug info
                    try
                    {
                        file pe(utf16_name);
                        dwarf temp(mod, pe);
                        temp.swap(dw);
                    }
                    catch (const std::exception &ex)
                    {
                        (void)ex;
                    }

                    // It's possible that a module may be unloaded and a different one loaded at the same
                    // address (especially since each has a preferred load address).
                    // So we bump the reference count to ensure it's not unloaded as long as we're around.

                    HMODULE temp = 0;
                    own_ref = GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, 
                                                 reinterpret_cast<const char *>(mod),
                                                 &temp);

                    assert(own_ref == false || mod == temp);
                }

                ~module_node()
                {
                    if (own_ref)
                        FreeLibrary(mod);
                }

                const char *name() const { return utf8_name; }

                const char *function_spanning(const void *program_counter) const
                { 
                    return dw.function_spanning(program_counter); 
                }

                static const module_node *find(HMODULE needle, const module_node *head)
                {
                    for ( ; head; head = head->next)
                        if (head->mod == needle) 
                            return head;
                    
                    return 0;
                }

                static void destroy_chain(const module_node *head)
                {
                    while (head)
                    {
                        const module_node *next = head->next;
                        delete head;
                        head = next;
                    }
                }
                
            private:
                HMODULE mod;
                char utf8_name[3 * 32 * 1024];
                bool own_ref;
                dwarf dw;
                const module_node *next;
        };

    } // anonymous

    struct symdb::impl
    {
        impl() : head(0) { }
        ~impl() { module_node::destroy_chain(head); }

        const module_node *get_node(HMODULE module)
        {
            if (!module) return 0;

            const module_node *node = module_node::find(module, head);
            if (!node)
            {
                node = new (std::nothrow) module_node(module, head);
                if (node) head = node;
            }

            return node;
        }

        ms_symdb msdb;
        const module_node *head;
    };


    symdb::symdb() :
        p(new (std::nothrow) impl)
    {
    }

    symdb::~symdb()
    {
        delete p;
    }

    bool symdb::lookup_function(const void *program_counter, symsink &sink) const
    {
        if (p)
        {
            // Try using Microsoft's symbol lookup mechanism first, as the module
            // might be have been built with MSVC.

            if (p->msdb.try_lookup_function(program_counter, sink))
                return true;

            MEMORY_BASIC_INFORMATION mbinfo;
            std::memset(&mbinfo, 0, sizeof mbinfo);

            // Use VirtualQuery to find the address at which the module containing the program_counter
            // was loaded.

            if (VirtualQuery(program_counter, &mbinfo, static_cast<DWORD>(sizeof mbinfo)) == sizeof mbinfo)
            {
                const module_node *node = p->get_node(static_cast<HMODULE>(mbinfo.AllocationBase));
                if (node)
                {
                    sink.on_function(program_counter, node->function_spanning(program_counter), node->name());
                    return true;
                }
            }
        }

        sink.on_function(program_counter, 0, 0);
        return false;
    }


} // dbg
