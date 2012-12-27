// Copyright Edd Dawson 2012
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <stdexcept>

#if defined(DBG_DISABLE_EXCEPTION_LOG)
#undef DBG_DISABLE_EXCEPTION_LOG
#endif

#if !defined(DBG_ENABLE_EXCEPTION_LOG)
#define DBG_ENABLE_EXCEPTION_LOG 1
#endif

#include "dbg/throw.hpp"

void die() { DBG_THROW(std::runtime_error("thrown with DBG_THROW")); }

void h() { die(); }
void g() { h(); }
void f() { g(); }

int main()
{
    f();
    return 0;
}
