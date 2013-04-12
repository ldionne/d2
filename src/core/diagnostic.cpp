/**
 * This file implements the `plain_text_explanation` function.
 */

#define D2_SOURCE
#include <d2/core/diagnostic.hpp>
#include <d2/detail/cyclic_permutation.hpp>

#include <boost/assert.hpp>
#include <boost/mem_fn.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <boost/range/algorithm/adjacent_find.hpp>
#include <boost/range/algorithm/sort.hpp>
#include <boost/range/algorithm/transform.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/spirit/include/karma.hpp>
#include <iterator>
#include <ostream>
#include <string>
#include <vector>


namespace karma = boost::spirit::karma;

namespace d2 {
namespace diagnostic_detail {
D2_DECL bool potential_deadlock::has_duplicate_threads() const {
    std::vector<ThreadId> thread_ids;
    thread_ids.reserve(threads.size());
    boost::transform(threads, std::back_inserter(thread_ids),
                     boost::mem_fn(&deadlocked_thread::tid));
    boost::sort(thread_ids);
    return boost::adjacent_find(thread_ids) != thread_ids.end();
}

D2_DECL std::ostream&
operator<<(std::ostream& os, deadlocked_thread const& self) {
    os << "{thread: " << self.tid << ", "
       << "holding: {" << karma::format(karma::stream % ", ", self.holding) << "}, "
       << "waiting for: " << self.waiting_for << "}";
    return os;
}

D2_DECL std::ostream&
operator<<(std::ostream& os, potential_deadlock const& self) {
    os << "{\n"
       << karma::format(karma::stream % ",\n", self.threads)
       << "\n}";
    return os;
}

D2_DECL bool
potential_deadlock::is_equivalent_to(potential_deadlock const& other) const {
    return detail::is_cyclic_permutation(threads, other.threads);
}

namespace {
std::string explain_thread(deadlocked_thread const& thread) {
    typedef deadlocked_thread::lock_sequence::size_type size_type;
    std::stringstream ss;

    ss << "in thread #" << thread.tid << " started at "
                                            << "[no location information]:\n";

    for (size_type i = 0; i < thread.holding.size(); ++i) {

        ss << "holds object #" << thread.holding.at(i) << " acquired at";
        if (thread.holding_info.at(i))
            ss << "\n" << *thread.holding_info.at(i) << "\n\n";
        else
            ss << " [no location information]\n";
    }

    ss << "tries to acquire object #" << thread.waiting_for << " at";
    if (thread.waiting_for_info)
        ss << "\n" << *thread.waiting_for_info << "\n\n";
    else
        ss << " [no location information]\n";

    return ss.str();
}
} // end anonymous namespace

D2_DECL extern std::ostream&
plain_text_explanation(std::ostream& os, potential_deadlock const& dl) {
    os << karma::format(karma::string % '\n',
                    dl.threads | boost::adaptors::transformed(explain_thread));
    return os;
}
} // end namespace diagnostic_detail
} // end namespace d2
