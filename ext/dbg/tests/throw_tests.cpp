// Copyright Edd Dawson 2012
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <test_o_matic.hpp>

#if defined(DBG_DISABLE_EXCEPTION_LOG)
#undef DBG_DISABLE_EXCEPTION_LOG
#endif

#if !defined(DBG_ENABLE_EXCEPTION_LOG)
#define DBG_ENABLE_EXCEPTION_LOG 1
#endif

#include "fixtures.hpp"

#include "dbg/throw.hpp"
#include "dbg/frames.hpp"

#include "fungo/catcher.hpp"
#include "fungo/exception_cage.hpp"

#include <stdexcept>
#include <cstring>
#include <sstream>

namespace
{
    bool streq(const char *lhs, const char *rhs)
    {
        return std::strcmp(lhs, rhs) == 0;
    }

    struct ohno { };

    struct fixture : envvar_fixture_base
    {
        fixture() : envvar_fixture_base("DBG_LOG_ON_THROW", "0") { }
    };

} // anonymous

TESTFIX("throw_info: current_exception() returns empty throw_info when no exception is in flight", fixture)
{
    {
        dbg::throw_info info = dbg::current_exception();
        CHECK(info.empty());
    }

    try { DBG_THROW(std::runtime_error("fish in power socket")); }
    catch (...) { }

    {
        dbg::throw_info info = dbg::current_exception();
        CHECK(info.empty());
    }
}

TESTFIX("throw_info: exceptions can be stored in a fungo::exception_cage", fixture)
{
    fungo::catcher c;
    fungo::exception_cage cage;

    try { DBG_THROW(std::runtime_error("fish")); }
    catch (...) { c.store_current_exception(cage); }

    CHECK(!cage.empty());
    THROWS(cage.rethrow(), std::runtime_error);
}

TESTFIX("throw_info: current_exception() contains data about throw site", fixture)
{
    unsigned line = 0;

    try 
    { 
        line = __LINE__ + 1;
        DBG_THROW(std::runtime_error("fish")); 
    }
    catch (...)
    {
        const dbg::throw_info info = dbg::current_exception();

        CHECK(!info.empty());
        CHECK(info.line == line);
        CHECK(streq(info.function, DBG_FUNCTION));
        CHECK(streq(info.file, __FILE__));
        CHECK(streq(info.expression, "std::runtime_error(\"fish\")"));
        CHECK(streq(info.what, "fish"));
        REQUIRE(info.trace != 0);
        CHECK(info.trace->size() != 0);
    }

    try
    {
        DBG_THROW(ohno());
    }
    catch (...)
    {
        const dbg::throw_info info = dbg::current_exception();

        // Not an std::exception, so no what() to store.
        CHECK(streq(info.what, ""));
        CHECK(streq(info.expression, "ohno()"));
    }
}

TESTFIX("throw_info: formatting of empty object", fixture)
{
    dbg::throw_info info;
    std::ostringstream oss;

    dbg::log(info, "bacon", oss);

    const std::string s = oss.str();

    CHECK(s.find("bacon") < s.find('\n')); // should be in header
    CHECK(s.find("location: \n") != std::string::npos);
    CHECK(s.find("function: \n") != std::string::npos);
    CHECK(s.find("expression: \n") != std::string::npos);
    CHECK(s.find("details: -\n") != std::string::npos);
    CHECK(s.find("[no call frames available]") != std::string::npos);
}

TESTFIX("throw_info: formatting of non-empty object", fixture)
{
    dbg::throw_info info;
    std::ostringstream oss;
    unsigned line = 0;

    try 
    { 
        line = __LINE__ + 1;
        DBG_THROW(std::runtime_error("fish")); 
    }
    catch (...)
    {
        info = dbg::current_exception();
    }

    dbg::log(info, "bacon", oss);

    const std::string s = oss.str();

    CHECK(s.find("bacon") < s.find('\n')); // should be in header
    CHECK(s.find("location: " __FILE__ ":" + to_string(line)) != std::string::npos);
    CHECK(s.find("function: " + std::string(DBG_FUNCTION)) != std::string::npos);
    CHECK(s.find("expression: std::runtime_error(\"fish\")") != std::string::npos);
    CHECK(s.find("details: fish\n") != std::string::npos);

    // should have some call frames.
    CHECK(s.find("[no call frames available]") == std::string::npos);
    CHECK(s.find("0x") != std::string::npos);
}

TESTFIX("throw_info: details are logged at throw site only when DBG_LOG_ON_THROW is in environment", fixture)
{
    THROWS(DBG_THROW(ohno()), ohno);
    CHECK(log.empty());

    set_envvar("1");

    unsigned line = __LINE__ + 1;
    THROWS(DBG_THROW(std::runtime_error("fish")), std::runtime_error);

    CHECK(log_contains("location: " __FILE__ ":" + to_string(line)));
    CHECK(log_contains("function: " + std::string(DBG_FUNCTION)));
    CHECK(log_contains("expression: std::runtime_error(\"fish\")"));
    CHECK(log_contains("details: fish\n"));

    // should have some call frames.
    CHECK(!log_contains("[no call frames available]"));
    CHECK(log_contains("0x"));
}
