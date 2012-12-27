// Copyright Edd Dawson 2010
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXCEPTIONS_HPP_2229_14102010
#define EXCEPTIONS_HPP_2229_14102010

#include <fungo/policies.hpp>
#include <exception>
#include <new>
#include <memory>

namespace fungo
{
    class exception;

#ifdef __llvm__
    // Workaround for linker errors from Apple's llvm-g++ 4.2
    typedef std::auto_ptr_ref< ::fungo::exception> auto_ptr_exception;
#else
    typedef std::auto_ptr< ::fungo::exception> auto_ptr_exception;
#endif

    //! A base class for all exceptions defined by fungo.
    class exception
    {
        public:
            virtual ~exception() throw() { }

            //! Returns a dynamically allocated copy of this exception whose dymamic type is
            //! the same as that of *this. Derived classes should override do_clone() to
            //! to modify the behaviour of this function.
            std::auto_ptr<exception> clone() const;

            //! Throws a copy of this exception whose dynamic type is the same as that of 
            //! *this. Derived classes should override do_throw_copy() to modify the behaviour
            //! of this function.
            void throw_copy() const;

        private:
            //! Derived classes should implement this to return a copy of this exception 
            //! with the same dynamic type.
            virtual auto_ptr_exception do_clone() const = 0;

            //! Derived classes should implement this to throw an object with the same 
            //! dynamic type as *this.
            virtual void do_throw_copy() const = 0;
    };

    //! A specialization of fungo::clone_policy that allows fungo to catch, clone and 
    //! re-throw all fungo exceptions polymorphically.
    template<>
    struct clone_policy<exception>
    {
        static exception *clone(const exception &ex) { return ex.clone().release(); }
        static void rethrow(const exception &ex) { ex.throw_copy(); }
    };


    //! An exception that is used to indicate another kind of exception couldn't be cloned.
    class clone_failure : public std::bad_alloc, public fungo::exception
    {
        public:
            clone_failure();
            ~clone_failure() throw();

            //! Returns a blurb which might help with debugging.
            const char *what() const throw();

        private:
            auto_ptr_exception do_clone() const;
            void do_throw_copy() const;
    };


    //! An exception that is used to indicate that something was caught, the type of which
    //! could not be determined.
    class unknown_exception : public std::exception, public fungo::exception
    {
        public:
            unknown_exception();
            ~unknown_exception() throw();

            //! Returns a blurb which might help with debugging.
            const char *what() const throw();

        private:
            auto_ptr_exception do_clone() const;
            void do_throw_copy() const;
    };

} // close namespace fungo

#endif // EXCEPTIONS_HPP_2229_14102010

