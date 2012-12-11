/**
 * This file implements platform-dependent routines to support the call stack
 * inteface.
 */

#ifndef D2_BTRACE_DETAIL_CALL_STACK_HPP
#define D2_BTRACE_DETAIL_CALL_STACK_HPP

#include <d2/btrace/detail/config.hpp>

#include <algorithm>
#include <boost/assert.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <cstddef>
#include <iterator>
#include <string>
#include <vector>


#if defined(D2_BTRACE_CONFIG_MACOSX)
#include <cstdlib>
#include <execinfo.h>

namespace d2 {
namespace btrace {
namespace detail {

template <typename T, typename OutputIterator>
OutputIterator transform_to_symbols(T* first, T* last, OutputIterator out) {
    typedef typename std::iterator_traits<T*>::difference_type Difference;
    Difference n = std::distance(first, last);
    char** buf = ::backtrace_symbols(&first, n);
    std::copy(buf, buf + n, out);
    std::free(buf);
    return out;
}

inline std::string get_symbol_at(void* address) {
    char** buf = ::backtrace_symbols(&address, 1);
    std::string ret(buf[0]);
    std::free(buf);
    return ret;
}

template <typename OutputIterator>
OutputIterator copy_frame_pointers(std::size_t max_frames,
                                   OutputIterator out) {
    BOOST_ASSERT_MSG(max_frames > 0, "max_frames must be greater than 0");
    std::vector<void*> bt(max_frames);
    std::size_t const size = ::backtrace(&bt[0], max_frames);
    return std::copy(boost::begin(bt), boost::begin(bt) + size, out);
}

} // end namespace detail
} // end namespace btrace
} // end namespace d2

#else
#   error "Your platform is not supported right now."
#endif

#endif // !D2_BTRACE_DETAIL_CALL_STACK_HPP
