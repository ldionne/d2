/**
 * This file defines alternative machinery to declare named template
 * parameters because of a bug in Boost.Parameter that might take a while
 * before it is fixed.
 *
 * @see https://svn.boost.org/trac/boost/ticket/2793
 */

#ifndef D2_SANDBOX_PARAMETER_HPP
#define D2_SANDBOX_PARAMETER_HPP

#include <boost/parameter.hpp>


namespace d2 {
namespace sandbox {

namespace parameter_detail {
    template <typename T>
    struct template_parameter_wrapper { };

    template <typename T>
    struct unwrap_template_parameter {
        typedef T type;
    };

    template <typename T>
    struct unwrap_template_parameter<template_parameter_wrapper<T> > {
        typedef T type;
    };
} // end namespace parameter_detail

template <typename Parameters, typename Keyword,
          typename Default = boost::parameter::void_>
struct parameter_value_type
    : parameter_detail::unwrap_template_parameter<
        typename boost::parameter::value_type<
            Parameters, Keyword, Default
        >::type
    >
{ };

template <typename Tag, typename T>
struct parameter_template_keyword
    : boost::parameter::template_keyword<
        Tag, parameter_detail::template_parameter_wrapper<T>
    >
{ };

#define D2_PARAMETER_TEMPLATE_KEYWORD(keyword)                              \
    namespace tag { struct keyword; }                                       \
    template <typename T>                                                   \
    struct keyword                                                          \
        : ::d2::sandbox::parameter_template_keyword<                        \
            tag::keyword, T                                                 \
        >                                                                   \
    { };                                                                    \
/**/

} // end namespace sandbox
} // end namespace d2

#endif // !D2_SANDBOX_PARAMETER_HPP
