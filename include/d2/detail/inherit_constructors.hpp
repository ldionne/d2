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
        D2_I_INHERIT_LVALUES(DERIVED, BASE)                                 \
    /**/

#   if defined(BOOST_NO_CXX11_RVALUE_REFERENCES)
#       define D2_I_INHERIT_LVALUES(DERIVED, BASE)                          \
            template <typename ...Args>                                     \
            explicit DERIVED(Args& ...args)                                 \
                : BASE(args...)                                             \
            { }                                                             \
        /**/
#   else
#       define D2_I_INHERIT_LVALUES(DERIVED, BASE) /* nothing */
#   endif

// If we don't have any of this
#else

#   define D2_I_CONSTRUCTORS(DERIVED, BASE, MAKE_REFERENCE_TYPE, FORWARD)   \
        template <typename A1>                                              \
        explicit DERIVED(MAKE_REFERENCE_TYPE(A1) a1)                        \
            : BASE(FORWARD(A1, a1))                                         \
        { }                                                                 \
                                                                            \
        template <typename A1, typename A2>                                 \
        explicit DERIVED(MAKE_REFERENCE_TYPE(A1) a1,                        \
                         MAKE_REFERENCE_TYPE(A2) a2)                        \
            : BASE(FORWARD(A1, a1), FORWARD(A2, a2))                        \
        { }                                                                 \
                                                                            \
        template <typename A1, typename A2, typename A3>                    \
        explicit DERIVED(MAKE_REFERENCE_TYPE(A1) a1,                        \
                         MAKE_REFERENCE_TYPE(A2) a2,                        \
                         MAKE_REFERENCE_TYPE(A3) a3)                        \
            : BASE(FORWARD(A1, a1), FORWARD(A2, a2), FORWARD(A3, a3))       \
        { }                                                                 \
/**/

#   define D2_I_MAKE_LVALUE_REF(T) T&
#   define D2_I_NO_FORWARD(T, t) t

#   define D2_I_MAKE_FWD_REF(T) BOOST_FWD_REF(T)
#   define D2_I_FORWARD(T, t) ::boost::forward<T>(t)

#   define D2_INHERIT_CONSTRUCTORS(DERIVED, BASE)                           \
        D2_I_CONSTRUCTORS(DERIVED, BASE, D2_I_MAKE_FWD_REF, D2_I_FORWARD)   \
        D2_I_INHERIT_LVALUES(DERIVED, BASE)                                 \
    /**/

#   ifdef BOOST_NO_CXX11_RVALUE_REFERENCES
#       define D2_I_INHERIT_LVALUES(DERIVED, BASE)                          \
            D2_I_CONSTRUCTORS(DERIVED, BASE,                                \
                              D2_I_MAKE_LVALUE_REF, D2_I_NO_FORWARD)        \
        /**/
#   else
#       define D2_I_INHERIT_LVALUES(DERIVED, BASE) /* nothing */
#   endif

#endif // end feature availability switch

#endif // !D2_DETAIL_INHERIT_CONSTRUCTORS_HPP
