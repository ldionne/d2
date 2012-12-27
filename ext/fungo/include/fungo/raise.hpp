// Copyright Edd Dawson 2010
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef RAISE_HPP_2328_16102010
#define RAISE_HPP_2328_16102010

#include <fungo/detail/implement_exception.hpp>

namespace fungo
{
    //! Throw a copy of the specified exception object that is also catchable as a 
    //! fungo::exception. Implemented by throwing an instance of a class that inherits both 
    //! Exception and fungo::exception (unless Exception already derives from fungo::exception,
    //! in which case ex.rethrow() is called).
    //!
    //! Since every fungo::catcher comes with the fungo::exception hierarchy pre-registered, 
    //! calling raise() rather than using a throw statement ensures that a catcher will be able
    //! to store your exception perfectly, regardless of its type.
    template<typename Exception>
    FUNGO_NO_RETURN void raise(const Exception &ex)
    {
        static const bool ife = detail::is_fungo_exception<Exception>::value;
        detail::raise_impl<Exception, ife>::raise(ex);
    }

} // close namespace fungo

#endif // RAISE_HPP_2328_16102010

