// Copyright Edd Dawson 2010
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <test_o_matic.hpp>
#include <fungo/exceptions.hpp>
#include <typeinfo>

TEST("clone_failure: can be caught as a fungo::exception")
{
    THROWS( throw fungo::clone_failure(), fungo::exception );
}

TEST("clone_failure: can be caught as an std::bad_alloc")
{
    THROWS( throw fungo::clone_failure(), std::bad_alloc );
}

TEST("clone_failure: clone() returns a clone_failure")
{
    fungo::clone_failure cf;
    fungo::exception &ex = cf;

    std::auto_ptr<fungo::exception> clone(ex.clone());

    CHECK( typeid(*clone) == typeid(fungo::clone_failure) );
}

TEST("clone_failure: throw_copy() throws a clone_failure")
{
    fungo::clone_failure cf;
    fungo::exception &ex = cf;
    
    THROWS( ex.throw_copy(), fungo::clone_failure );
}


TEST("unknown_exception: can be caught as a fungo::exception")
{
    THROWS( throw fungo::unknown_exception(), fungo::exception );
}

TEST("unknown_exception: can be caught as an std::exception")
{
    THROWS( throw fungo::unknown_exception(), std::exception );
}

TEST("unknown_exception: clone() returns a unknown_exception")
{
    fungo::unknown_exception cf;
    fungo::exception &ex = cf;

    std::auto_ptr<fungo::exception> clone(ex.clone());

    CHECK( typeid(*clone) == typeid(fungo::unknown_exception) );
}

TEST("unknown_exception: throw_copy() throws a unknown_exception")
{
    fungo::unknown_exception cf;
    fungo::exception &ex = cf;
    
    THROWS( ex.throw_copy(), fungo::unknown_exception );
}
