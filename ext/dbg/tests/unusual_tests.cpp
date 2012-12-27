// Copyright Edd Dawson 2012
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <test_o_matic.hpp>

#if defined(DBG_DISABLE_UNUSUAL)
#undef DBG_DISABLE_UNUSUAL
#endif

#if !defined(DBG_ENABLE_UNUSUAL)
#define DBG_ENABLE_UNUSUAL 1
#endif

#include "dbg/unusual.hpp"
#include "fixtures.hpp"
#include "utils.hpp"

namespace
{
    std::string softdrinks_failure()
    {
        int coke = 1;
        int pepsi = 1;

        DBG_UNUSUAL(coke == pepsi);

        return DBG_FUNCTION;
    }

} // anonymous

TESTFIX("DBG_UNUSUAL: nothing is done when condition is false", assert_fixture)
{
    DBG_UNUSUAL(1 == 2);

    CHECK(log.empty());
    CHECK(fatalities_seen == 0);
}

TESTFIX("DBG_UNUSUAL: fatality function is not called, even when condition is true", assert_fixture)
{
    DBG_UNUSUAL(1 == 1);
    CHECK(fatalities_seen == 0);
}

TESTFIX("DBG_UNUSUAL: the unusual condition is mentioned", assert_fixture)
{
    int coke = 1;
    int pepsi = 1;

    DBG_UNUSUAL(coke == pepsi);

    CHECK(log_contains("condition: coke == pepsi"));
}

TESTFIX("DBG_UNUSUAL: the source code position is mentioned", assert_fixture)
{
    int coke = 1;
    int pepsi = 1;

    DBG_UNUSUAL(coke == pepsi);
    CHECK( log_contains("location: " __FILE__ ":" + to_string(__LINE__ - 1)) );
}

TESTFIX("DBG_UNUSUAL: the failing function is mentioned", assert_fixture)
{
    std::string func = softdrinks_failure();
    CHECK(log_contains("function: " + func));
}

TESTFIX("DBG_UNUSUAL: the call stack is mentioned", assert_fixture)
{
    softdrinks_failure();

    // This isn't the greatest test in the world, but it's very hard to reliably 
    // stop compilers optimizing out certain functions.

    CHECK(log_contains("    0x")); // start of a PC address
}

TESTFIX("DBG_UNUSUAL: DBG_UTAGged variables are included in failure output", assert_fixture)
{
    int coke = 1;
    int pepsi = 1;
    std::string animal = "kestrel";

    DBG_UNUSUAL(coke == pepsi), DBG_UTAG(coke), DBG_UTAG(pepsi), DBG_UTAG(animal);

    CHECK(log_contains("'coke': 1"));
    CHECK(log_contains("'pepsi': 1"));
    CHECK(log_contains("'animal': kestrel"));
}
