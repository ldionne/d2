// Copyright Edd Dawson 2010
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <fungo/catcher.hpp>
#include <fungo/exception_cage.hpp>
#include <fungo/exceptions.hpp>

#include <cassert>

namespace fungo
{
    namespace
    {
        void null_destroy(void *exc) 
        {
            assert(exc == reinterpret_cast<void *>(0xbad));
            (void)exc;
        } 

        template<typename Exception> 
        void hurl(const void *) { throw Exception(); }

        struct cage_data
        {
            void (*rethrow)(const void *);
            void (*destroy_clone)(void *);
            void *exception;
        };

        cage_data make_dummy_cage_data(void (*rethrow)(const void *))
        {
            cage_data data = { rethrow, &null_destroy, reinterpret_cast<void *>(0xbad) };
            return data;
        }

        cage_data make_cage_data(const detail::glove &g, void *exc)
        {
            if (exc)
            {
                // The capture function of the glove managed to clone the exception without complication.
                cage_data data = { g.rethrow, g.destroy_clone, exc };
                return data; 
            }

            // Now the special case. When the glove's capture function attempted to clone the exception 
            // it caught, that resulted in another exception.
            return make_dummy_cage_data(&hurl<clone_failure>);
        }

        cage_data attempt_capture_with_glove(const std::vector<detail::glove> &gloves, std::size_t n, 
                                             bool propagate_clone_exception)
        {
            const detail::glove &g = gloves[n];

            if (n == 0)
            {
                // We're at the bottom of the recursive calls.
                // The g.capture() call here may leak an exception. As the stack unwinds, higher calls to
                // attempt_capture_with_glove() may catch that exception.
                return make_cage_data(g, g.capture(propagate_clone_exception)); 
            }
            else
            {
                try { return attempt_capture_with_glove(gloves, n - 1, propagate_clone_exception); }
                catch (...) { return make_cage_data(g, g.capture(propagate_clone_exception)); }
            }
        }

    } // close anonymous namespace


    catcher::catcher()
    {
        learn<fungo::exception>();
    }

    void catcher::store_current_exception(exception_cage &cage) const
    {
        if (gloves_.empty()) throw;
        
        const cage_data data = attempt_capture_with_glove(gloves_, gloves_.size() - 1, true);

        exception_cage temp;
        temp.rethrow_ = data.rethrow;
        temp.destroy_clone_ = data.destroy_clone;
        temp.exception_ = data.exception;

        assert(!temp.empty());

        temp.swap(cage);
    }

    void catcher::store_current_exception(exception_cage &cage, std::nothrow_t) const
    {
        exception_cage temp;
        cage_data data;

        try
        {
            if (gloves_.empty()) throw;
            data = attempt_capture_with_glove(gloves_, gloves_.size() - 1, false);
        }
        catch (...)
        {
            data = make_dummy_cage_data(&hurl<unknown_exception>);
        }

        temp.rethrow_ = data.rethrow;
        temp.destroy_clone_ = data.destroy_clone;
        temp.exception_ = data.exception;

        assert(!temp.empty());

        cage.swap(temp);
    }

    void catcher::learn(const detail::glove &g, void (*ptr_thrower)())
    {
        typedef std::vector<detail::glove>::iterator iter_t;
        iter_t it = gloves_.begin(), e = gloves_.end();

        if (g.specialized) 
        {
            // If g is specialized, it's a good bet that it can handle its particular type of exception
            // better than any other glove, so we put specialized gloves near the front.
            while (it != e)
            {
                if (!it->specialized) break; // at the back of the specialized gloves
                if (it->capture == g.capture) return; // we already have this glove.

                ++it;
            }
        }
        else
        {
            // Otherwise, we seek an appropriate place to put g among the existing unspecialized gloves.
            while (it != e)
            {
                // Since g isn't specialized, we'll make sure to put it after any existing specialized gloves
                // at the front.
                if (it->specialized) { ++it; continue; }

                // Do we already have the glove?
                if (it->capture == g.capture) return;

                // Beyond that, we can be fairly confident that ordering gloves by the sizeof their exceptions,
                // biggest to smallest, is a good idea because bigger exceptions will typically be more derived
                // than smaller ones. Thus trying to catch bigger exception types first will result in less
                // slicing when copying them using the copy constructor of the caught type.
                if (it->size > g.size) { ++it; continue; }

                // If the current glove has a smaller .size than that of g, we're at the point of insertion.
                if (it->size < g.size) break;

                assert(it->size == g.size);


                // Can *it recognise the exception? If so, we'll make sure to insert g before *it in the 
                // list. This will also help prevent slicing when copying caught exceptions. For example, an 
                // existing glove for std::exception will be able to catch an std::runtime_error, but we need to
                // make sure std::runtime_error's glove is tried before std::exception's.
                assert(ptr_thrower);

                try { ptr_thrower(); }
                catch (...)
                {
                    if (it->recognise())
                        break;
                }

                ++it;
            }
        }

        gloves_.insert(it, g);
    }

    void catcher::swap(catcher &other)
    {
        gloves_.swap(other.gloves_);
    }

    void swap(catcher &c1, catcher &c2)
    {
        c1.swap(c2);
    }

} // close namespace fungo
