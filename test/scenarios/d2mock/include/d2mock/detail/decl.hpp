/**
 * This file defines the `D2MOCK_DECL` macro.
 */

#ifndef D2MOCK_DETAIL_DECL_HPP
#define D2MOCK_DETAIL_DECL_HPP

#include <boost/config.hpp>


#if defined(D2MOCK_DYN_LINK)
#   if defined(D2MOCK_SOURCE)
#       define D2MOCK_DECL BOOST_SYMBOL_EXPORT
#   else
#       define D2MOCK_DECL BOOST_SYMBOL_IMPORT
#   endif
#else
#   define D2MOCK_DECL /* nothing */
#endif

#endif // !D2MOCK_DETAIL_DECL_HPP
