/**
 * This file contains configuration options and macros to stay portable.
 */

#ifndef D2_DETAIL_CONFIG_HPP
#define D2_DETAIL_CONFIG_HPP

#include <boost/config.hpp>

#if defined(D2_DYN_LINK)
#   if defined(D2_SOURCE)
#       define D2_DECL BOOST_SYMBOL_EXPORT
#   else
#       define D2_DECL BOOST_SYMBOL_IMPORT
#   endif
#else
#   define D2_DECL /* nothing */
#endif

#endif // !D2_DETAIL_CONFIG_HPP
