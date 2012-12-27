// Copyright Edd Dawson 2012
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#if defined(DBG_DISABLE_ASSERT)
#undef DBG_DISABLE_ASSERT
#endif

#if !defined(DBG_ENABLE_ASSERT)
#define DBG_ENABLE_ASSERT 1
#endif

//#define DBG_ENABLE_HOT_ASSERT

#include "dbg/assert.hpp"
#include "dbg/fatality.hpp"

#include <iostream>

int calc(int i)
{
    return i;
}

void oops()
{
    const int a = 7;
    const int b = 9;
    const int c = DBG_IMPURE_ASSERT(calc(5)); // fine

    DBG_ASSERT(c == 5); // fine

    DBG_HOT_ASSERT(a == b), DBG_HOT_TAG(a), DBG_HOT_TAG(b); // when DBG_ENABLE_HOT_ASSERT: bang!

    DBG_ASSERT(a == b), DBG_TAG(a), DBG_TAG(b); // bang!

    DBG_ASSERT_RELATION(a, >=, b); // bang!

    DBG_ASSERT_UNREACHABLE; // bang!
}

void h() { oops(); }
void g() { h(); }
void f() { g(); }

void noop() { }

int main()
{
    // Here we make triggered dbg assertions non-fatal for demo purposes.
    dbg::set_fatality_handler(noop);

    f();
    return 0;
}
