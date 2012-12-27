// Copyright Edd Dawson 2010
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <fungo/exceptions.hpp>
#include <typeinfo>
#include <cassert>

namespace fungo
{
    std::auto_ptr<exception> exception::clone() const
    {
        std::auto_ptr<exception> ret(do_clone());

#if !defined(NDEBUG)
        assert(ret.get());
        assert(typeid(*this) == typeid(*ret)); // check dynamic type of clone
#endif

        return ret;
    }

    void exception::throw_copy() const
    {
#if defined(NDEBUG)
        do_throw_copy();
#else
        try 
        { 
            do_throw_copy(); 

            // do_throw_copy() does not appear to have been implemented correctly.
            // It hasn't thrown anything at all.
            assert(false);
        }
        catch (const exception &ex)
        {
            assert(typeid(ex) == typeid(*this)); // check dynamic type of thrown copy
            throw;
        }
        catch (...) 
        { 
            // do_throw_copy() does not appear to have been implemented correctly.
            // It's throwing something that isn't even a fungo::exception.
            assert(false); 
        }
#endif
    }



    clone_failure::clone_failure() :
        std::bad_alloc()
    {
    }

    clone_failure::~clone_failure() throw()
    {
    }

    const char *clone_failure::what() const throw()
    {
        return "An unknown exception was thrown while cloning an exception";
    }

    auto_ptr_exception clone_failure::do_clone() const
    {
        return std::auto_ptr<fungo::exception>(new clone_failure(*this));
    }

    void clone_failure::do_throw_copy() const
    {
        throw clone_failure(*this);
    }



    unknown_exception::unknown_exception() :
        std::exception()
    {
    }

    unknown_exception::~unknown_exception() throw()
    {
    }

    const char *unknown_exception::what() const throw()
    {
        return "An unknown exception occurred";
    }

    auto_ptr_exception unknown_exception::do_clone() const
    {
        return std::auto_ptr<fungo::exception>(new unknown_exception(*this));
    }

    void unknown_exception::do_throw_copy() const
    {
        throw unknown_exception(*this);
    }

} // close namespace fungo

