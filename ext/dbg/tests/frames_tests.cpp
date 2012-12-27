// Copyright Edd Dawson 2012
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <test_o_matic.hpp>

#include "dbg/frames.hpp"
#include "dbg/static_assert.hpp"

#include <cstring>

namespace
{
    typedef dbg::call_stack<5> call_stack;

    DBG_NEVER_INLINE void f8(call_stack &cs, unsigned ignore) { cs.collect(ignore); }
    DBG_NEVER_INLINE void f7(call_stack &cs, unsigned ignore) { f8(cs, ignore); }
    DBG_NEVER_INLINE void f6(call_stack &cs, unsigned ignore) { f7(cs, ignore); }
    DBG_NEVER_INLINE void f5(call_stack &cs, unsigned ignore) { f6(cs, ignore); }
    DBG_NEVER_INLINE void f4(call_stack &cs, unsigned ignore) { f5(cs, ignore); }
    DBG_NEVER_INLINE void f3(call_stack &cs, unsigned ignore) { f4(cs, ignore); }
    DBG_NEVER_INLINE void f2(call_stack &cs, unsigned ignore) { f3(cs, ignore); }
    DBG_NEVER_INLINE void f1(call_stack &cs, unsigned ignore) { f2(cs, ignore); }
    DBG_NEVER_INLINE void f0(call_stack &cs, unsigned ignore) { f1(cs, ignore); }
}

// FIXME: May be susceptible to failure due to aggressive compiler optimizations.
// Could put each function in its own translation unit, but there's global optimization...
TEST("frames: callstack collection [fragile]")
{
    call_stack cs0;
    call_stack cs1;

    f0(cs0, 1);
    f0(cs1, 2);

    REQUIRE(cs0.size() >= 2);
    REQUIRE(cs1.size() >= 1);

    CHECK(cs0.pc(1) == cs1.pc(0));
}
