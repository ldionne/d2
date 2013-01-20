/**
 * This file contains configuration options and macros to stay portable.
 *
 * If we have access to boost, use their macros to achieve maximum
 * portability. Otherwise, roll our own macros and support some common cases.
 *
 * To signal that boost is available, define the `D2_HAS_BOOST` macro before
 * including this header.
 *
 * @internal This header is included in the public API, hence the care to keep
 *           it compatible with C (no C++ comments!) and not to take for
 *           granted the availability of boost.
 */

#ifndef D2_DETAIL_CONFIG_HPP
#define D2_DETAIL_CONFIG_HPP

#if defined(_WIN32) || defined(__WIN32__) || defined(WIN32)
#   define D2_WIN32
#endif

#if defined(D2_HAS_BOOST)
#   include <boost/config.hpp>
#   define D2_SYMBOL_IMPORT BOOST_SYMBOL_IMPORT
#   define D2_SYMBOL_EXPORT BOOST_SYMBOL_EXPORT
#else
#   if defined(D2_WIN32)
#       define D2_SYMBOL_IMPORT __declspec(dllimport)
#       define D2_SYMBOL_EXPORT __declspec(dllexport)
#   else
#       define D2_SYMBOL_IMPORT /* nothing */
#       define D2_SYMBOL_EXPORT /* nothing */
#   endif
#endif /* defined(D2_HAS_BOOST) */


#if defined(D2_DYN_LINK)
#   if defined(D2_SOURCE)
#       define D2_API D2_SYMBOL_EXPORT
#   else
#       define D2_API D2_SYMBOL_IMPORT
#   endif
#else
#   define D2_API /* nothing */
#endif

#endif /* !D2_DETAIL_CONFIG_HPP */
