/**
 * This file defines the `unordered_difference` algorithm.
 */

#ifndef D2_DETAIL_UNORDERED_DIFFERENCE_HPP
#define D2_DETAIL_UNORDERED_DIFFERENCE_HPP

#include <algorithm>
#include <boost/move/utility.hpp>
#include <boost/next_prior.hpp>
#include <boost/phoenix/bind.hpp>
#include <boost/phoenix/core/argument.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <set>


namespace d2 {
namespace unordered_difference_detail {
/**
 * @internal
 * Return an iterator to the first element for which `pred` returns true
 * and which is not in a set of `consumed` iterators.
 */
template <typename Iterator, typename Consumed, typename UnaryPredicate>
Iterator find_next_unconsumed_if(Iterator first, Iterator last,
                                 Consumed& consumed,
                                 UnaryPredicate const& pred) {
    Iterator it = std::find_if(first, last, pred);
    while (it != last) {
        // If we already consumed it, try finding the next unconsumed element.
        if (!consumed.insert(it).second)
            it = std::find_if(boost::next(it), last, pred);
        // Otherwise, we found the next unconsumed element.
        else
            return it;
    }
    return it;
}

/**
 * @internal
 * Same thing as `find_next_unconsumed_if`, but using `operator==`
 * for comparison.
 */
template <typename Iterator, typename Consumed, typename Value>
Iterator find_next_unconsumed(Iterator first, Iterator last,
                              Consumed& consumed, Value const& value) {
    Iterator it = std::find(first, last, value);
    while (it != last) {
        // If we already consumed it, try finding the next unconsumed element.
        if (!consumed.insert(it).second)
            it = std::find(boost::next(it), last, value);
        // Otherwise, we found the next unconsumed element.
        else
            return it;
    }
    return it;
}

/**
 * Perform a set-like difference of two ranges on which no partial order
 * is defined.
 *
 * This algorithm has the same effect as `std::set_difference`, but it does
 * not require the input ranges to be sorted in any way, and the
 * `BinaryPredicate` represents equality rather than strict weak ordering.
 *
 * @note The iterators of the range delimited by `[first2, last2)` are
 *       required to be random access iterators.
 */
template <typename Iterator1, typename Iterator2,
          typename OutputIterator, typename BinaryPredicate>
OutputIterator
unordered_difference(Iterator1 first1, Iterator1 last1,
                     Iterator2 first2, Iterator2 last2,
                     OutputIterator result, BinaryPredicate pred) {
    std::set<Iterator2> consumed;
    while (first1 != last1) {
        Iterator2 it = find_next_unconsumed_if(first2, last2, consumed,
                        boost::phoenix::bind(pred, *first1,
                                             boost::phoenix::arg_names::_1));
        if (it == last2)
            *result++ = *first1;
        ++first1;
    }
    return result;
}

//! Version of `unordered_difference` using `operator==` as a comparator.
template <typename Iterator1, typename Iterator2, typename OutputIterator>
OutputIterator unordered_difference(Iterator1 first1, Iterator1 last1,
                                    Iterator2 first2, Iterator2 last2,
                                    OutputIterator result) {
    std::set<Iterator2> consumed;
    while (first1 != last1) {
        Iterator2 it = find_next_unconsumed(first2, last2, consumed, *first1);
        if (it == last2)
            *result++ = *first1;
        ++first1;
    }
    return result;
}

//! Version of `unordered_difference` taking ranges.
template <typename Range1, typename Range2, typename OutputIterator>
OutputIterator unordered_difference(Range1 const& range1, Range2 const& range2,
                                    BOOST_FWD_REF(OutputIterator) result) {
    return unordered_difference(boost::begin(range1), boost::end(range1),
                                boost::begin(range2), boost::end(range2),
                                boost::forward<OutputIterator>(result));
}

//! Version of `unordered_difference` taking ranges.
template <typename Range1, typename Range2,
          typename OutputIterator, typename BinaryPredicate>
OutputIterator unordered_difference(Range1 const& range1, Range2 const& range2,
                                    BOOST_FWD_REF(OutputIterator) result,
                                    BOOST_FWD_REF(BinaryPredicate) pred) {
    return unordered_difference(boost::begin(range1), boost::end(range1),
                                boost::begin(range2), boost::end(range2),
                                boost::forward<OutputIterator>(result),
                                boost::forward<BinaryPredicate>(pred));
}
} // end namespace unordered_difference_detail

namespace detail {
    using unordered_difference_detail::unordered_difference;
}
} // end namespace d2

#endif // !D2_DETAIL_UNORDERED_DIFFERENCE_HPP
