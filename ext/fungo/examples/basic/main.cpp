// Copyright Edd Dawson 2010
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <fungo/catcher.hpp>
#include <fungo/exception_cage.hpp>

#include <stdexcept>
#include <iostream>

struct base 
{ 
    base(const char *message) : message(message) { }
    
    const char *message;
};

struct derived : base
{ 
    derived() : base("derived exception") { }
};

int main()
{
    // When fungo tries to catch and store the current exception, it will try to catch 
    // more-derived exception types first as a means to prevent slicing.

    // Let's see if using catcher::learn() allows the catcher to order the
    // catch attempts correctly.
    
    fungo::catcher catcher1;
    catcher1.learn<base>();
    catcher1.learn<derived>();

    fungo::exception_cage cage;

    try { throw derived(); }
    catch (...) { catcher1.store_current_exception(cage); }

    // an exception is now stored in cage

    try { cage.rethrow(); }
    catch (const derived &ex)
    {
        std::cout << "caught derived: " << ex.message << '\n';
    }


    // We'll do the same again, but register the exceptions in the reverse order to show 
    // that registration order is irrelevant.
    cage.clear();

    fungo::catcher catcher2;
    catcher2.learn<derived>();
    catcher2.learn<base>();

    try { throw derived(); }
    catch (...) { catcher2.store_current_exception(cage); }

    // an exception is now stored in cage

    try { cage.rethrow(); }
    catch (const derived &ex)
    {
        std::cout << "caught derived: " << ex.message << '\n';
    }

    return 0;
}
