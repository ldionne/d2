// Copyright Edd Dawson 2012
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#if defined(DBG_DISABLE_EXCEPTION_LOG)
#undef DBG_DISABLE_EXCEPTION_LOG
#endif

#if !defined(DBG_ENABLE_EXCEPTION_LOG)
#define DBG_ENABLE_EXCEPTION_LOG 1
#endif

#include "dbg/throw.hpp"

#include <stdexcept>
#include <iostream>

void hurl()
{
    DBG_THROW(std::runtime_error("thrown by DBG_THROW"));
}

void h() { hurl(); }
void g() { h(); }
void f() { g(); }

int main()
{
    try
    {
        f();
    }
    catch (...)
    {
        const dbg::throw_info info = dbg::current_exception();

        if (!info.empty())
        {
            // When thrown with DBG_THROW, we can get the call stack
            // of the throw point and some other tidbits.

            dbg::log(info, "Caught exception!", std::cout);
        }
    }

    return 0;
}
