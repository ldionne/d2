/*!
 * @file
 * This file defines the `d2::detail::ut_access` class.
 */

#ifndef D2_DETAIL_UT_ACCESS_HPP
#define D2_DETAIL_UT_ACCESS_HPP

namespace d2 {
namespace detail {
/*!
 * @internal
 * Class used to access the internals of some types in `d2` for the
 * sake of scenario testing.
 */
class ut_access {
public:
    template <typename T>
    static std::size_t d2_unique_id(T const& t) {
        return t.d2_unique_id();
    }
};
} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_UT_ACCESS_HPP
