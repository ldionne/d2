// Copyright Edd Dawson 2012
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef FIXTURES_HPP_1139_26082012
#define FIXTURES_HPP_1139_26082012

#include "dbg/fatality.hpp"
#include "dbg/log.hpp"

#include "utils.hpp"

#include <string>

// Sets/restores a logsink (this) in setup/teardown that writes to a string.
struct sink_fixture : dbg::logsink
{
    sink_fixture() : old_sink(dbg::set_logsink(self())) { }
    ~sink_fixture() { dbg::set_logsink(old_sink); }

    dbg::logsink &self() { return *this; } // workaround MSVC warning about the use of 'this' in intializer list

    virtual void write(const char *text, std::size_t n) { log.insert(log.end(), text, text + n); }
    bool log_contains(const std::string &needle) const { return log.find(needle) != std::string::npos; }

    dbg::logsink &old_sink;
    std::string log;
};

// Sets/restores fatality_handler in setup/teardown that counts fatalities seen, but does
// not terminate the process.
struct assert_fixture : sink_fixture
{
    assert_fixture() : old_handler(dbg::set_fatality_handler(assert_fixture::on_fatality)) { fatalities_seen = 0; }
    ~assert_fixture() { dbg::set_fatality_handler(old_handler); }

    static unsigned fatalities_seen;
    static void on_fatality() { ++fatalities_seen; }

    const dbg::fatality_handler old_handler;
};

namespace dbg { bool envvar_is_set(const char *name); } // internal dbg function

// Initializes a particular environment variable in setup
struct envvar_fixture_base : sink_fixture
{
    envvar_fixture_base(const char *varname, const char *initial) : 
        varname(varname),
        was_set(dbg::envvar_is_set(varname))
    {
        ::set_envvar(varname, initial);
    }

    ~envvar_fixture_base()
    {
        ::set_envvar(varname, was_set ? "1" : "");
    }

    void set_envvar(const std::string &value) { ::set_envvar(varname, value.c_str()); }

    const char * const varname;
    const bool was_set;
};

#endif // FIXTURES_HPP_1139_26082012
