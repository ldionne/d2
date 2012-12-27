// Copyright Edd Dawson 2012
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#if defined(DBG_DISABLE_NOTE)
#undef DBG_DISABLE_NOTE
#endif

#if !defined(DBG_ENABLE_NOTE)
#define DBG_ENABLE_NOTE 1
#endif

#include "dbg/note.hpp"

#include <cstdlib>
#include <iostream>

int main()
{
    const char *ev = std::getenv("DBG_ENABLE_NOTE");

    if (!ev || ev[0] == '\0' || ev[0] == '0')
        std::cout << "DBG_ENABLE_NOTE must be set in the environment for notes to appear\n";

    const int a = 42;
    DBG_NOTE(a);

    const char *name = "Fred";
    DBG_NOTE(name);

    DBG_NOTE("hello");

    return 0;
}
