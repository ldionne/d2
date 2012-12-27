// Copyright Edd Dawson 2010
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef CATCHER_HPP_1936_12102010
#define CATCHER_HPP_1936_12102010

#include <fungo/policies.hpp>
#include <fungo/detail/glove.hpp>
#include <vector>
#include <new>

namespace fungo
{
    class exception_cage;


    //! A catcher is a repository of exception identification and handling functionality.
    //! By telling a catcher about particular exception types (or base classes of clone-able
    //! exception hierarchies), it will be able to catch and store a copy of such exceptions.
    class catcher
    {
        public:
            //! Creates a catcher that initially only knows about the exceptions in the 
            //! fungo::exception hierarchy.
            catcher();

            //! Let this catcher know about the given Exception type so that it may be caught at
            //! a later time by store_current_exception(). If Exception is the base class in a 
            //! hierarchy of clone-able exception types, you will probably want to specialize 
            //! the fungo::clone_policy template for Exception.
            template<typename Exception>
            void learn()
            {
                learn(detail::make_glove<Exception>(), &detail::opaque_pointer_thrower<Exception>);
            }

            //! This must be called inside a catch block:
            //!
            //! \code
            //! try { task(); }
            //! catch (...) { my_catcher.store_current_exception(my_cage); }
            //! \endcode
            //!
            //! The current exception is rethrown and an attempt is made to recognise that 
            //! exception and store it in \a cage. 
            //!
            //! If the exception is not recognised the rethrown exception is propagated.
            //! Similarly, if the act of copying/cloning a recognised exception itself throws an 
            //! exception, it will be propagated unless recognised by a subsequent internal catch
            //! attempt.
            void store_current_exception(exception_cage &cage) const;

            //! This must be called inside a catch block:
            //!
            //! \code
            //! try { task(); }
            //! catch (...) { my_catcher.store_current_exception(my_cage, std::nothrow); }
            //! \endcode
            //!
            //! Behaves like the other overload, except that no exception will be propagated 
            //! under any circumstances.
            //!
            //! Instead, either a fungo::unknown_exception or fungo::clone_failure will be placed
            //! into \a cage, depending on the cause of the exception.
            void store_current_exception(exception_cage &cage, std::nothrow_t) const;

            //! Exchanges the state of this object with that of other.
            void swap(catcher &other);

        private:
            void learn(const detail::glove &g, void (*ptr_thrower)());

        private:
            std::vector<detail::glove> gloves_;
    };

    //! Equivalent to c1.swap(c2).
    void swap(catcher &c1, catcher &c2);

} // close namespace fungo

#endif // CATCHER_HPP_1936_12102010

