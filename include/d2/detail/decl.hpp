/**
 * This file defines the `D2_DECL` macro.
 */

#ifndef D2_DETAIL_DECL_HPP
#define D2_DETAIL_DECL_HPP

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

#endif // !D2_DETAIL_DECL_HPP
