/*!
 * This file defines the `D2_DECL` macro.
 *
 * Prior to including this file, define the `D2_API_NO_BOOST` if Boost is
 * not available when including this header.
 *
 * @internal
 * If we have access to Boost, use their macros to achieve maximum
 * portability. Otherwise, roll our own macros and support some common cases.
 *
 * IMPORTANT NOTE:
 * Do not use C++ only features in this header because the pure C API includes
 * it too!
 */

#ifndef D2_DETAIL_DECL_HPP
#define D2_DETAIL_DECL_HPP

#if !defined(D2_API_NO_BOOST)

#   include <boost/config.hpp>
#   define D2_SYMBOL_EXPORT BOOST_SYMBOL_EXPORT
#   define D2_SYMBOL_IMPORT BOOST_SYMBOL_IMPORT

#else /* no support for Boost */

#   if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#       define D2_WIN32
#   endif
#   if defined(D2_WIN32)
#       define D2_SYMBOL_IMPORT __declspec(dllimport)
#       define D2_SYMBOL_EXPORT __declspec(dllexport)
#   else
#       define D2_SYMBOL_IMPORT /* nothing */
#       define D2_SYMBOL_EXPORT /* nothing */
#   endif
#endif

#if defined(D2_DYN_LINK)
#   if defined(D2_SOURCE)
#       define D2_DECL D2_SYMBOL_EXPORT
#   else
#       define D2_DECL D2_SYMBOL_IMPORT
#   endif
#else
#   define D2_DECL /* nothing */
#endif

#endif /* !D2_DETAIL_DECL_HPP */
