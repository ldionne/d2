/**
 * This file defines a macro to generate a metafunction testing whether a type
 * supports a given expression.
 */

#ifndef D2_DETAIL_SUPPORTS_EXPRESSION_HPP
#define D2_DETAIL_SUPPORTS_EXPRESSION_HPP

#include <boost/mpl/bool.hpp>


#define D2_DETAIL_SUPPORTS_EXPRESSION(name, expression)                     \
template <typename T>                                                       \
class name {                                                                \
    struct yes { char size[1]; };                                           \
    struct no { char size[2]; };                                            \
                                                                            \
    template <typename C>                                                   \
    static yes test(char[sizeof(expression)]);                              \
                                                                            \
    template <typename C>                                                   \
    static no test(...);                                                    \
                                                                            \
public:                                                                     \
    typedef typename ::boost::mpl::bool_<                                   \
                        sizeof(test<T>(0)) == sizeof(yes)                   \
                >::type type;                                               \
    static bool const value = type::value;                                  \
}                                                                           \
/**/

#endif // !D2_DETAIL_SUPPORTS_EXPRESSION_HPP
