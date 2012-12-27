// Copyright Edd Dawson 2010
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef GLOVE_HPP_1938_12102010
#define GLOVE_HPP_1938_12102010

#include <fungo/policies.hpp>
#include <cstddef>

namespace fungo
{
    namespace detail
    {
        // Contains all the know-how needed to catch a particular type or base-type of exception,
        // clone it and rethrow that clone.
        // It's not some polymorphic base class, but rather a bundle of function pointers. This 
        // eliminates the need to dynamically allocate derived gloves to handle different types of
        // exception. This is important, since dynamic allocation can result in an std::bad_alloc,
        // and we'd rather have the true exception come through rather than that.
        struct glove
        {
            // Some sensible default state. All 0s.
            glove();

            glove(void *(*capture)(bool),
                  bool (*recognise)(),
                  void (*rethrow)(const void *),
                  void (*destroy_clone)(void *),
                  bool specialized,
                  std::size_t size);

            // bool argument determines whether exceptions thrown during clone should be propagated. 0 is
            // returned if the bool is false but an exception was thrown during the clone operation.
            void *(*capture)(bool); 

            // Returns true if the capture() function could catch and clone a copy of the current exception.
            bool (*recognise)();

            // Throws the exception previously caught by capture()
            void (*rethrow)(const void *opaque_exception);

            // Deletes/destroys the exception object previously caught by capture()
            void (*destroy_clone)(void *opaque_exception);

            // True if fungo::clone_policy was specialized for the associated exception type.
            bool specialized;

            // The sizeof the associated exception class. Used for sorting gloves in to an appropriate order
            // for capture() attempts.
            std::size_t size;
        };
        
        // Throws a null-valued pointer-to-Exception
        template<typename Exception>
        void opaque_pointer_thrower()
        {
            throw static_cast<Exception *>(0);
        }


        // A function with the signature of glove::recognise that returns true if an Exception object is 
        // currently caught.
        template<typename Exception>
        bool opaque_recognise()
        {
            try { throw; }
            catch (Exception *) { return true; }
            catch (...) { return false; }
        }

        // A function with the signature of glove::capture that attempts to rethrow the currently caught 
        // exception, catch it as an Exception and then clone it using the fungo::clone_policy for Exception.
        template<typename Exception>
        void *opaque_capture(bool propagate_clone_exception) 
        {
            try { throw; }
            catch (const Exception &ex)
            {
                try { return ::fungo::clone_policy<Exception>::clone(ex); }
                catch (...) { if (propagate_clone_exception) throw; } 
            }

            return 0;
        }

        // A function with the signature of glove::rethrow that casts \a exception to an Exception and throws
        // a copy of it using the fungo::clone_policy for Exception.
        template<typename Exception>
        void opaque_rethrow(const void *opaque_exception) 
        {
            const Exception &real = *static_cast<const Exception *>(opaque_exception);
            return ::fungo::clone_policy<Exception>::rethrow(real);
        }

        // A function with the signature of glove::destroy_clone that casts \a exception to an Exception and
        // destroys it using the fungo::destroy_clone_policy for Exception.
        template<typename Exception>
        void opaque_destroy_clone(void *opaque_exception) 
        {
            Exception *realp = static_cast<Exception *>(opaque_exception);
            return ::fungo::destroy_clone_policy<Exception>::destroy_clone(realp);
        }

        template<typename Exception>
        glove make_glove()
        {
            const bool clone_specialized = is_specialized< ::fungo::clone_policy<Exception> >::value;

            return glove(&opaque_capture<Exception>,
                         &opaque_recognise<Exception>,
                         &opaque_rethrow<Exception>,
                         &opaque_destroy_clone<Exception>,
                         clone_specialized,
                         sizeof(Exception));
        }

    } // close namespace detail

} // close namespace fungo

#endif // GLOVE_HPP_1938_12102010

