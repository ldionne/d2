// Copyright Edd Dawson 2012
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <test_o_matic.hpp>

#include "dbg/log.hpp"
#include "dbg/logstream.hpp"

#include "fixtures.hpp"

namespace
{
    struct dummy_sink : dbg::logsink
    {
        virtual void write(const char *, std::size_t) { }
    };

} // anonymous

TESTFIX("log: set_logsink() returns old logsink", sink_fixture)
{
    dummy_sink sink;
    CHECK(&dbg::set_logsink(sink) == this);
    CHECK(&dbg::set_logsink(*this) == &sink);
}

TESTFIX("log: log_write() writes n characters from string to current sink", sink_fixture)
{
    dbg::log_write("abcd", 3);
    CHECK(log == "abc");
}

TESTFIX("log: inserting characters in to a log_stream writes to the current sink", sink_fixture)
{
    {
        dbg::logstream out;
        out << "hello" << 123;

        // out's destructor should flush.
    }

    CHECK(log == "hello123");
}
