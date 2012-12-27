// Copyright Edd Dawson 2012
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <test_o_matic.hpp>
#include <iostream>

namespace tom { using namespace test_o_matic; }

int main()
{
    tom::simple_logger lgr(std::cout, false);
    tom::runner rnr;

    for (const tom::test *t = tom::first_test(); t; t = t->next)
        tom::run_test(*t, lgr, rnr);

    return lgr.summary(std::cout);
}
