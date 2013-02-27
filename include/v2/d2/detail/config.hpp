/**
 * This file contains configuration options and macros to ease writing
 * portable code.
 */

#ifndef D2_DETAIL_CONFIG_HPP
#define D2_DETAIL_CONFIG_HPP

#include <boost/config.hpp>


#define D2_SYMBOL_IMPORT BOOST_SYMBOL_IMPORT
#define D2_SYMBOL_EXPORT BOOST_SYMBOL_EXPORT

#if defined(D2_DYN_LINK)
#   if defined(D2_SOURCE)
#       define D2_DECL D2_SYMBOL_EXPORT
#   else
#       define D2_DECL D2_SYMBOL_IMPORT
#   endif
#else
#   define D2_DECL /* nothing */
#endif

#endif // !D2_DETAIL_CONFIG_HPP
