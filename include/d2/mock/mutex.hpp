/**
 * This file defines a mock mutex.
 */

#ifndef D2_MOCK_MUTEX_HPP
#define D2_MOCK_MUTEX_HPP

#include <d2/detail/config.hpp>

#include <boost/noncopyable.hpp>
#include <cstddef>


namespace d2 {
namespace mock {

class mutex : boost::noncopyable {
    std::size_t id_;

public:
    D2_API mutex();

    D2_API void lock();

    D2_API void unlock();

    D2_API friend std::size_t unique_id(mutex const& self);
};

} // end namespace mock
} // end namespace d2

#endif // !D2_MOCK_MUTEX_HPP
