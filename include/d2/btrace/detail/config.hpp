/**
 * This file is used to set some flags providing information about the
 * platform we are being compiled on.
 */

#ifndef D2_BTRACE_DETAIL_CONFIG_HPP
#define D2_BTRACE_DETAIL_CONFIG_HPP

#if defined(macintosh) || defined(Macintosh)
#   define D2_BTRACE_CONFIG_MACOS9
#elif (__APPLE__ & __MACH__)
#   define D2_BTRACE_CONFIG_MACOSX
#endif

#endif // !D2_BTRACE_DETAIL_CONFIG_HPP
