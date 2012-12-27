// Copyright Edd Dawson 2012
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef UTILS_HPP_1347_26082012
#define UTILS_HPP_1347_26082012

#include <sstream>
#include <cstdlib>

inline std::string to_string(unsigned n)
{
    std::ostringstream oss;
    oss << n;
    return oss.str();
}

#if defined(_WIN32)
extern "C" int __stdcall SetEnvironmentVariableA(const char *name, const char *value);

inline void set_envvar(const char *name, const char *value)
{
    SetEnvironmentVariableA(name, value);
}
#else
inline void set_envvar(const char *name, const char *value)
{
    setenv(name, value, 1);
}
#endif

#endif // UTILS_HPP_1347_26082012
