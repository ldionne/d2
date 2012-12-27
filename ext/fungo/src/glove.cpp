// Copyright Edd Dawson 2010
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <fungo/detail/glove.hpp>

namespace fungo 
{
     namespace detail 
     {
        glove::glove() :
            capture(0),
            recognise(0),
            rethrow(0),
            destroy_clone(0),
            specialized(false),
            size(0)
        {
        }

        glove::glove(void *(*capture)(bool),
                     bool (*recognise)(),
                     void (*rethrow)(const void *),
                     void (*destroy_clone)(void *),
                     bool specialized,
                     std::size_t size) :
            capture(capture),
            recognise(recognise),
            rethrow(rethrow),
            destroy_clone(destroy_clone),
            specialized(specialized),
            size(size)
        {
        }
     
     } // close namespace detail

} // close namespace fungo
