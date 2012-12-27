// Copyright Edd Dawson 2010
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <test_o_matic.hpp>
#include <fungo/fungo.hpp>
#include <stdexcept>

TEST("raise: unregistered exception type can be caught by catcher if raise()d")
{
    fungo::catcher mitten;
    fungo::exception_cage cage;

    try { fungo::raise(std::runtime_error("oops")); }
    catch (...)
    {
        mitten.store_current_exception(cage);
        THROWS( cage.rethrow(), std::runtime_error );
        THROWS( cage.rethrow(), fungo::exception );
    }
}

TEST("raise: works correctly with existing fungo::exceptions")
{
    fungo::catcher mitten;
    fungo::exception_cage cage;

    try { fungo::raise(fungo::unknown_exception()); }
    catch (...)
    {
        mitten.store_current_exception(cage);
        THROWS( cage.rethrow(), fungo::unknown_exception );
        THROWS( cage.rethrow(), fungo::exception );
    }
}
