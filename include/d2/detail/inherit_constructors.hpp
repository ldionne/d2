/**
 * This file defines the `D2_INHERIT_CONSTRUCTORS` macro.
 */

#ifndef D2_DETAIL_INHERIT_CONSTRUCTORS_HPP
#define D2_DETAIL_INHERIT_CONSTRUCTORS_HPP

#include <boost/config.hpp>
#include <boost/move/utility.hpp>


// If we have inherited constructors
// Note: BOOST_NO_CXX11_INHERITED_CONSTRUCTORS does not currently exist,
//       so we always disable this.
#if !defined(BOOST_NO_CXX11_INHERITED_CONSTRUCTORS) && 0

#   define D2_INHERIT_CONSTRUCTORS(DERIVED, BASE)                           \
        using D2_BASE_CLASS::D2_BASE_CLASS;                                 \
    /**/

// If we have variadic templates
#elif !defined(BOOST_NO_CXX11_VARIADIC_TEMPLATES)

#   define D2_INHERIT_CONSTRUCTORS(DERIVED, BASE)                           \
        template <typename ...Args>                                         \
        explicit DERIVED(BOOST_FWD_REF(Args) ...args)                       \
            : BASE(::boost::forward<Args>(args)...)                         \
        { }                                                                 \
                                                                            \
        D2_IMPL_INHERIT_LVALUES(DERIVED, BASE)                              \
    /**/

#   if defined(BOOST_NO_CXX11_RVALUE_REFERENCES)                            \
#       define D2_IMPL_INHERIT_LVALUES(DERIVED, BASE)                       \
            template <typename ...Args>                                     \
            explicit DERIVED(Args& ...args)                                 \
                : BASE(args...)                                             \
            { }                                                             \
        /**/
#   else
#       define D2_IMPL_INHERIT_LVALUES(DERIVED, BASE) /* nothing */
#   endif

// If we don't have any of this
#else

#   define D2_INHERIT_CONSTRUCTORS(DERIVED, BASE)                           \
        DERIVED() { }                                                       \
                                                                            \
        template <typename A1>                                              \
        explicit DERIVED(BOOST_FWD_REF(A1) a1)                              \
            : BASE(::boost::foward<A1>(a1))                                 \
        { }                                                                 \
                                                                            \
        D2_IMPL_INHERIT_LVALUES(DERIVED, BASE)                              \
    /**/

#   ifdef BOOST_NO_CXX11_RVALUE_REFERENCES
#       define D2_IMPL_INHERIT_LVALUES(DERIVED, BASE)                       \
            template <typename A1>                                          \
            explicit DERIVED(A1& a1)                                        \
                : BASE(a1)                                                  \
            { }                                                             \
        /**/
#   else
#       define D2_IMPL_INHERIT_LVALUES(DERIVED, BASE) /* nothing */
#   endif

#endif // end feature availability switch

#endif // !D2_DETAIL_INHERIT_CONSTRUCTORS_HPP
