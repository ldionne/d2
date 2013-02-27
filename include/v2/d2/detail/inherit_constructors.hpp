/**
 * This file can be included to inherit the constructors of a base class.
 *
 * Before inclusion, the `D2_BASE_CLASS` and the `D2_DERIVED_CLASS` macros
 * must be defined and must represent the obvious. These macros will be
 * automatically undefined by this file.
 */

#ifndef D2_BASE_CLASS
#   error "the D2_BASE_CLASS macro must be defined before including this file"
#endif

#ifndef D2_DERIVED_CLASS
#   error "the D2_DERIVED_CLASS macro must be defined before including this file"
#endif

using D2_BASE_CLASS::D2_BASE_CLASS;

#undef D2_BASE_CLASS
#undef D2_DERIVED_CLASS


// The following not completely implemented yet.
#if 0
#if !defined(D2_NO_CXX11_INHERITING_CONSTRUCTORS)

    using D2_BASE_CLASS::D2_BASE_CLASS;

#elif defined(D2_NO_CXX11_VARIADIC_TEMPLATES)

    D2_DERIVED_CLASS() { }

    template <typename A1>
    explicit D2_DERIVED_CLASS(D2_FWD_REF(A1) a1)
        : D2_BASE_CLASS(::d2::detail::foward<A1>(a1))
    { }

#       ifdef D2_NO_CXX11_RVALUE_REFERENCES
            template <typename A1>
            explicit D2_DERIVED_CLASS(A1& a1)
                : D2_BASE_CLASS(a1)
            { }
#       endif

#else // we have variadic templates

    template <typename ...Args>
    explicit D2_DERIVED_CLASS(D2_FWD_REF(Args) ...args)
        : D2_BASE_CLASS(::d2::detail::forward<Args>(args)...)
    { }

#       ifdef D2_NO_CXX11_RVALUE_REFERENCES
            template <typename ...Args>
            explicit D2_DERIVED_CLASS(Args& ...args)
                : D2_BASE_CLASS(args...)
            { }
#       endif
#endif
#endif
