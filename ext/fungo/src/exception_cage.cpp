// Copyright Edd Dawson 2010
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <fungo/exception_cage.hpp>
#include <cassert>
#include <algorithm>

namespace fungo 
{
    exception_cage::exception_cage() :
        rethrow_(0),
        destroy_clone_(0),
        exception_(0)
    {
    }

    exception_cage::~exception_cage()
    {
        clear();
    }

    bool exception_cage::empty() const
    {
        return exception_ == 0;
    }

    void exception_cage::clear()
    {
        if (exception_)
        {
            assert(destroy_clone_);
            destroy_clone_(exception_);

            rethrow_ = 0;
            destroy_clone_ = 0;
            exception_ = 0;
        }

        assert(empty());
    }

    void exception_cage::rethrow() const
    {
        if (exception_)
        {
            assert(rethrow_);
            rethrow_(exception_);
        }
    }

    void exception_cage::rethrow_and_clear()
    {
        try { rethrow(); }
        catch (...)
        {
            clear();
            throw;
        }

        assert(empty());
    }

    void exception_cage::swap(exception_cage &other)
    {
        std::swap(rethrow_, other.rethrow_);
        std::swap(destroy_clone_, other.destroy_clone_);
        std::swap(exception_, other.exception_);
    }

    void swap(exception_cage &c1, exception_cage &c2)
    {
        c1.swap(c2);
    }

} // close namespace fungo
