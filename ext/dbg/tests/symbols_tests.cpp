// Copyright Edd Dawson 2012
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <test_o_matic.hpp>

#include "dbg/symbols.hpp"
#include "dbg/symlog.hpp"
#include "dbg/static_assert.hpp"

#include <string>
#include <cstring>

namespace
{
    const char *splendid() { return "splendid"; }

    struct sink : dbg::symsink
    {
        sink() : pc(0) { }

        virtual void process_function(const void *program_counter, const char *funcname, const char *modname)
        {
            pc = program_counter;
            function = funcname;
            module = modname;
        }

        const void *pc;
        std::string function;
        std::string module;
    };
}

#if !defined(_MSC_VER) || defined(DBG_RECENT_DBGHELP_DLL)
TEST("symbols: symbol lookup")
{
    dbg::symdb db;

    void *vfp = 0;
    const char *(*fp)() = &splendid;

    DBG_STATIC_ASSERT(sizeof vfp == sizeof fp);
    std::memcpy(&vfp, &fp, sizeof fp);

    sink s;

    CHECK(db.lookup_function(vfp, s));

    CHECK(s.pc == vfp);
    CHECK(s.module != "[unknown module]");
    CHECK(s.function.find("splendid") != std::string::npos);
}
#endif

TEST("symbols: symlog formatting")
{
    std::ostringstream oss;
    dbg::symlog sink(oss, "<PREFIX>", "<SUFFIX>");

    sink.on_function(&oss, "the_function()", "the_module");

    const std::string s = oss.str();

    const std::size_t colon_loc = ((sizeof "<PREFIX>0x") - 1) + sizeof(void*) * 2;

    CHECK(s.find("<PREFIX>0x") == 0);
    CHECK(s.find(": the_function() in the_module<SUFFIX>") == colon_loc);
}
