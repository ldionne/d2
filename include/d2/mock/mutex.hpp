/**
 * This file defines a mock mutex class.
 */

#ifndef D2_MOCK_MUTEX_HPP
#define D2_MOCK_MUTEX_HPP

#include <d2/api.hpp>
#include <d2/detail/config.hpp>

#include <boost/noncopyable.hpp>


namespace d2 {
namespace mock {

struct mutex : boost::noncopyable, public deadlock_detectable<mutex> {
    D2_API void lock();

    D2_API void unlock();
};

} // end namespace mock
} // end namespace d2

#endif // !D2_MOCK_MUTEX_HPP
