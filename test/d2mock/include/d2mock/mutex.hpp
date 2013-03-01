/**
 * This file defines the `mutex` class.
 */

#ifndef D2MOCK_MUTEX_HPP
#define D2MOCK_MUTEX_HPP

#include <d2mock/detail/decl.hpp>

#include <boost/noncopyable.hpp>
#include <d2/api.hpp>


namespace d2mock {
namespace mutex_detail {
struct mutex : boost::noncopyable, public d2::deadlock_detectable<mutex> {
    D2MOCK_DECL void lock();

    D2MOCK_DECL void unlock();
};
} // end namespace mutex_detail

using mutex_detail::mutex;
} // end namespace d2

#endif // !D2MOCK_MUTEX_HPP
