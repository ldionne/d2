// Copyright Edd Dawson 2010
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef POLICIES_HPP_2323_13102010
#define POLICIES_HPP_2323_13102010

#include <fungo/detail/unspecialized.hpp>

namespace fungo
{

    //! If you have an exception hierarchy, the classes of which have the means to copy 
    //! themselves polymorphically (via a clone() method or similar) and throw copies of
    //! themselves polymorphically, then you can inform fungo::catcher objects of this by
    //! specializing clone_policy for the base exception of that hierarchy.
    template<typename Exception>
    struct clone_policy : detail::unspecialized
    {
        //! Specializations should implement this to return a dynamically allocated copy of the
        //! given exception object. It is expected that the copy will have the same dynamic type
        //! as that of the argument. By default, any such clones will be deleted by fungo in the
        //! usual fashion. If an alternative method of destroying objects derived Exception 
        //! is required, you should also specialize destroy_clone_policy for Exception.
        static Exception *clone(const Exception &e) { return new Exception(e); }

        //! Specializations should implement this to throw a copy of the given exception. It
        //! is expected that the thrown exception will have the same dynamic type as that of the
        //! argument.
        static void rethrow(const Exception &e) { throw e; }
    };

    //! If you dynamically allocated Exception objects need to be destroyed in a special way
    //! (i.e. by some means other than the delete operator), then you should specialize this
    //! template for the Exception class.
    template<typename Exception>
    struct destroy_clone_policy
    {
        //! Specializations should implement this function to perform the analog to the delete
        //! operator; call the destructor of the given exception and release any allocated memory.
        static void destroy_clone(Exception *e) { delete e; }
    };


} // close namespace fungo

#endif // POLICIES_HPP_2323_13102010

