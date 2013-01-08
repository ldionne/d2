/**
 * This file defines a macro to generate a metafunction testing whether a type
 * supports a given expression.
 */

#ifndef D2_SANDBOX_SUPPORTS_EXPRESSION_HPP
#define D2_SANDBOX_SUPPORTS_EXPRESSION_HPP

#include <boost/mpl/bool.hpp>
#include <boost/preprocessor/cat.hpp>


#define D2_I_MAKE_UNIQUE_NAME(x) BOOST_PP_CAT(x, __LINE__)

#define D2_DETAIL_SUPPORTS_EXPRESSION(name, expression, typename_param)     \
template <typename D2_I_MAKE_UNIQUE_NAME(T)>                                \
class name {                                                                \
    struct yes { char size[1]; };                                           \
    struct no { char size[2]; };                                            \
                                                                            \
    template <typename_param>                                               \
    static yes test(char[sizeof(expression)]);                              \
                                                                            \
    template <typename_param>                                               \
    static no test(...);                                                    \
                                                                            \
public:                                                                     \
    typedef typename ::boost::mpl::bool_<                                   \
                    sizeof(test<D2_I_MAKE_UNIQUE_NAME(T)>(0)) == sizeof(yes)\
                >::type type;                                               \
    static bool const value = type::value;                                  \
}                                                                           \
/**/

#endif // !D2_SANDBOX_SUPPORTS_EXPRESSION_HPP
