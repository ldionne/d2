/**
 * This file implements the `plain_text_explanation` function.
 */

#define D2_SOURCE
#include <d2/core/diagnostic.hpp>

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
#include <set>
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
    // We just get rid of the order of the threads.
    typedef std::set<deadlocked_thread> DeadlockedThreads;
    return DeadlockedThreads(threads.begin(), threads.end()) ==
           DeadlockedThreads(other.threads.begin(), other.threads.end());
}

namespace {
std::string explain_thread(deadlocked_thread const& thread) {
    std::string ret;
    karma::generate(std::back_inserter(ret),
        "thread " << karma::stream << " holds " << (karma::stream % ", ")
                                   << " and waits for " << karma::stream,
        thread.tid, thread.holding, thread.waiting_for);
    return ret;
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
