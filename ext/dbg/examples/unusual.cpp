// Copyright Edd Dawson 2012
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#if defined(DBG_DISABLE_UNUSUAL)
#undef DBG_DISABLE_UNUSUAL
#endif

#if !defined(DBG_ENABLE_UNUSUAL)
#define DBG_ENABLE_UNUSUAL 1
#endif

#include "dbg/unusual.hpp"

#include <cstdlib>

int main()
{
    for (unsigned i = 0; i != 10000; ++i)
    {
        const int r = std::rand();
        DBG_UNUSUAL(r % 3000 == 0), DBG_UTAG(r);
    }
}
