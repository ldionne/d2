// Copyright Edd Dawson 2012
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <test_o_matic.hpp>

#if defined(DBG_DISABLE_NOTE)
#undef DBG_DISABLE_NOTE
#endif

#if !defined(DBG_ENABLE_NOTE)
#define DBG_ENABLE_NOTE 1
#endif

#include "dbg/note.hpp"
#include "dbg/log.hpp"

#include "fixtures.hpp"

#include <sstream>

namespace
{
    struct fixture : envvar_fixture_base
    {
        fixture() : envvar_fixture_base("DBG_ENABLE_NOTE", "1") { }
    };

} // anonymous

TESTFIX("DBG_NOTE: notes only appear when DBG_ENABLE_NOTE is in the environment", fixture)
{
    set_envvar("");

    DBG_NOTE("hello");
    CHECK(log.empty());

    set_envvar("0");

    DBG_NOTE("hello");
    CHECK(log.empty());

    set_envvar("1");

    DBG_NOTE("hello");
    CHECK(log_contains("hello"));
}

TESTFIX("DBG_NOTE: note contains source location", fixture)
{
    const unsigned line = __LINE__ + 1;
    DBG_NOTE("hello");

    CHECK(log_contains(__FILE__ ":" + to_string(line)));
}

TESTFIX("DBG_NOTE: note contains name of variable if argument is not a string literal", fixture)
{
    int x = 40;
    int y = 2;
    DBG_NOTE(x + y);
    CHECK(log_contains("x + y = 42"));
    
    log.clear();
    const char *name = "Fred";
    DBG_NOTE(name);
    CHECK(log_contains("name = Fred"));

    log.clear();
    DBG_NOTE("here");
    CHECK(!log_contains("="));
    CHECK(log_contains("here"));
}
