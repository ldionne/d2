// Copyright Edd Dawson 2010
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef EXCEPTION_CAGE_HPP_1222_16102010
#define EXCEPTION_CAGE_HPP_1222_16102010

namespace fungo
{

    //! A class for storing an exception that can be re-thrown at a later time.
    //! catcher::store_current_exception() is used to put the current exception inside an 
    //! exception_cage.
    class exception_cage
    {
        public:
            //! Create an empty exception_cage.
            exception_cage();

            ~exception_cage();

            //! Returns true if and only if no exception is stored within.
            bool empty() const;

            //! Clears any exception stored within. Post-condition: empty() == true.
            void clear();

            //! Rethrows any exception stored within. If no exception is stored, returns 
            //! immediately without any side effects.
            void rethrow() const; 

            //! Equivalent to:
            //! \code
            //! try { rethrow(); }
            //! catch (...) { clear(); throw; }
            //! \endcode
            void rethrow_and_clear();

            //! Exchanges the state of this object with that of \a other.
            void swap(exception_cage &other);

        private:
            //! Copying is forbidden
            exception_cage(const exception_cage &); 

            //! Copying is forbidden
            exception_cage &operator= (const exception_cage &); 

            friend class catcher;

        private:
            void (*rethrow_)(const void *);
            void (*destroy_clone_)(void *);
            void *exception_;
    };

    //! Equivalent to c1.swap(c2).
    void swap(exception_cage &c1, exception_cage &c2);


} // close namespace fungo

#endif // EXCEPTION_CAGE_HPP_1222_16102010

