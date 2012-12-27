// Copyright Edd Dawson 2012
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include "dbg/symbols.hpp"
#include "dbg/frames.hpp"

#include <iostream>

void traceback()
{
    dbg::call_stack<16> trace;
    trace.collect(1); // 1 so we don't capture traceback() itself.

    dbg::symdb db;
    trace.log(db, std::cout, "-> ");
}

void f0() { traceback(); }
void f1() { f0(); }
void f2() { f1(); }
void f3() { f2(); }
void f4() { f3(); }
void f5() { f4(); }

template<unsigned N> struct chain { static void call() { chain<N-1>::call(); } };
template<> struct chain<0> { static void call() { f5(); } };

int main()
{
    chain<5>::call();

    return 0;
}
