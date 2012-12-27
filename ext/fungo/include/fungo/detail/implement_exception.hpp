// Copyright Edd Dawson 2010
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef IMPLEMENT_EXCEPTION_HPP_2257_16102010
#define IMPLEMENT_EXCEPTION_HPP_2257_16102010

#include <fungo/exceptions.hpp>
#include <cstdlib>

#if defined(_MSC_VER) && _MSC_VER >= 1200
    #define FUNGO_NO_RETURN __declspec(noreturn)
#elif defined(__GNUC__) && __GNUC__ >= 3
    #define FUNGO_NO_RETURN __attribute__((noreturn))
#else
    #define FUNGO_NO_RETURN
#endif

namespace fungo
{
    namespace detail
    {
        // Inherits Exception and fungo::exception. Implements fungo::exception's virtual
        // interface too.
        template<typename Exception>
        struct implement_exception : public Exception, public ::fungo::exception
        {
            public:
                implement_exception(const Exception &ex) : Exception(ex) { }
                ~implement_exception() throw() { }

            private:
                virtual auto_ptr_exception do_clone() const
                {
                    std::auto_ptr< ::fungo::exception> ret(new implement_exception(*this));
                    return ret;
                }

                virtual void do_throw_copy() const 
                {
                    throw implement_exception(*this);
                }
        };

        // Meta-function to determine if Exception inherits fungo::exception
        template<typename Exception>
        struct is_fungo_exception
        {
            typedef char small;
            typedef char (&big)[2];

            static small test(...);
            static big test(const ::fungo::exception *);

            // N.B. can't use static const bool due to bug(?) in MSVC's LTCG.
            enum { value = sizeof test(static_cast<Exception *>(0)) == sizeof(big) };
        };

        // Implementation detail of fungo::raise()
        template<typename Exception, bool Rethrow>
        struct raise_impl
        {
            static FUNGO_NO_RETURN void raise(const Exception &ex) { throw implement_exception<Exception>(ex); }
        };

        // Implementation detail of fungo::raise()
        template<typename Exception>
        struct raise_impl<Exception, true>
        {
            static FUNGO_NO_RETURN void raise(const Exception &ex) 
            { 
                ex.throw_copy();
                std::abort(); // else function is seen to return by compiler => warning
            }
        };

    } // close namespace detail

} // close namespace fungo

#endif // IMPLEMENT_EXCEPTION_HPP_2257_16102010

