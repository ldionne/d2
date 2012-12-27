// Copyright Edd Dawson 2010
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef UNSPECIALIZED_HPP_2326_13102010
#define UNSPECIALIZED_HPP_2326_13102010

namespace fungo
{
    namespace detail
    {
        struct unspecialized { };

        // A meta-function that returns true if Policy does not inherit unspecialized.
        template<typename Policy>
        struct is_specialized
        {
            typedef char small;
            typedef char (&big)[2];

            static small test(...);
            static big test(unspecialized *);

            static const bool value = sizeof test(static_cast<Policy *>(0)) == 1;
        };

    } // close namespace detail

} // close namespace fungo

#endif // UNSPECIALIZED_HPP_2326_13102010

