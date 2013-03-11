/**
 * This file defines the `is_cyclic_permutation` algorithm.
 */

#ifndef D2_CORE_CYCLIC_PERMUTATION_HPP
#define D2_CORE_CYCLIC_PERMUTATION_HPP

#include <algorithm>
#include <boost/assert.hpp>
#include <boost/iterator/iterator_traits.hpp>
#include <boost/move/utility.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/next_prior.hpp>
#include <boost/range/algorithm/equal.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/range/join.hpp>
#include <boost/regex/v4/iterator_category.hpp>
#include <boost/utility/enable_if.hpp>
#include <iterator>
#include <utility>


namespace d2 {
namespace cyclic_permutation_detail {
/**
 * @internal
 * Tag used when we know that we're comparing ranges of the same length.
 */
struct ranges_have_the_same_length { };

//! @internal Tag used when we can't assume both ranges have the same length.
struct dont_know_if_same_length { };

/**
 * @internal
 * Metafunction returning whether it is O(1) to compute the distance between
 * two iterators.
 */
template <typename Iterator>
struct has_O1_distance
    : boost::mpl::bool_<
        ::boost::is_random_access_iterator<Iterator>::value
    >
{ };

//! @internal Perform a comparison between a range and another shifted range.
template <typename Iterator1, typename Iterator2>
bool compare_shifted(Iterator1 first1, Iterator1 last1,
                     Iterator2 first2, Iterator2 shift, Iterator2 last2,
                     dont_know_if_same_length) {
    // return whether [first1, last1) == [shift, last2) U [first2, shift)
    return boost::equal(
            boost::make_iterator_range(first1, last1),

            boost::join(
                boost::make_iterator_range(shift, last2),
                boost::make_iterator_range(first2, shift)));
}

/**
 * @internal
 * Overload of `compare_shifted` for when we know both ranges have
 * the same length.
 */
template <typename Iterator1, typename Iterator2>
bool compare_shifted(Iterator1 first1, Iterator1 /* last1 */,
                     Iterator2 first2, Iterator2 shift, Iterator2 last2,
                     ranges_have_the_same_length) {
    std::pair<Iterator2, Iterator1> const& mism =
                                        std::mismatch(shift, last2, first1);
    // Let n = distance(shift, last2).
    // We return whether the two following conditions are met:
    //  (1) [shift, last2) == [first1, first1 + n)
    //  (2) [first2, shift) == [first1 + n, last1)
    return mism.first == last2 && std::equal(first2, shift, mism.second);
}

//! @internal Core of the algorithm working on non empty ranges.
template <typename Iterator1, typename Iterator2, typename LengthKnowledge>
bool is_nonempty_cyclic_permutation(Iterator1 first1, Iterator1 last1,
                                    Iterator2 first2, Iterator2 last2,
                                    LengthKnowledge) {
    BOOST_ASSERT_MSG(first1 != last1, "[first1, last1) must be non-empty");
    BOOST_ASSERT_MSG(first2 != last2, "[first2, last2) must be non-empty");

    Iterator2 shift = std::find(first2, last2, *first1);
    while (shift != last2) {
        if (compare_shifted(first1, last1,
                            first2, shift, last2, LengthKnowledge()))
            return true;
        else
            shift = std::find(boost::next(shift), last2, *first1);
    }
    return false;
}

/**
 * Algorithm returning whether a range delimited by [`first1`, `last1`) is
 * a cyclic permutation of the range delimited by [`first2`, `last2`).
 *
 * The comparison between elements of the ranges is done using `operator==`.
 *
 * If both ranges are empty, the algorithm returns `true`.
 */
template <typename Iterator1, typename Iterator2>
typename boost::disable_if<
    boost::mpl::and_<
        has_O1_distance<Iterator1>, has_O1_distance<Iterator2>
    >,
bool>::type is_cyclic_permutation(Iterator1 first1, Iterator1 last1,
                                  Iterator2 first2, Iterator2 last2) {
    // [first1, last1) is empty
    if (first1 == last1)
        return first2 == last2;
    // [first2, last2) is empty, but [first1, last1) is not
    else if (first2 == last2)
        return false;
    else
        return is_nonempty_cyclic_permutation(first1, last1, first2, last2,
                                              dont_know_if_same_length());
}

//! Version of `is_cyclic_permutation` optimized for random access iterators.
template <typename Iterator1, typename Iterator2>
typename boost::enable_if<
    boost::mpl::and_<
        has_O1_distance<Iterator1>, has_O1_distance<Iterator2>
    >,
bool>::type is_cyclic_permutation(Iterator1 first1, Iterator1 last1,
                                  Iterator2 first2, Iterator2 last2) {
    typedef typename boost::iterator_difference<Iterator1>::type Distance1;
    typedef typename boost::iterator_difference<Iterator2>::type Distance2;

    using std::distance;
    Distance1 const dist1 = distance(first1, last1);
    Distance2 const dist2 = distance(first2, last2);

    if (dist1 != dist2)
        return false;
    // dist2 == 0 need not be checked because dist1 == dist2
    else if (dist1 == 0)
        return true;
    else
        return is_nonempty_cyclic_permutation(first1, last1, first2, last2,
                                              ranges_have_the_same_length());
}

//! Overload of `is_cyclic_permutation` taking a range as a first parameter.
template <typename Range, typename Iterator>
bool is_cyclic_permutation(Range const& range, BOOST_FWD_REF(Iterator) first,
                                               BOOST_FWD_REF(Iterator) last) {
    return is_cyclic_permutation(boost::begin(range), boost::end(range),
                                 boost::forward<Iterator>(first),
                                 boost::forward<Iterator>(last));
}

//! Overload of `is_cyclic_permutation` taking a range as a third parameter.
template <typename Iterator, typename Range>
bool is_cyclic_permutation(BOOST_FWD_REF(Iterator) first,
                           BOOST_FWD_REF(Iterator) last, Range const& range) {
    return is_cyclic_permutation(boost::forward<Iterator>(first),
                                 boost::forward<Iterator>(last),
                                 boost::begin(range), boost::end(range));
}

//! Overload of `is_cyclic_permutation` taking two ranges.
template <typename Range1, typename Range2>
bool is_cyclic_permutation(Range1 const& range1,
                           BOOST_FWD_REF(Range2) range2) {
    return is_cyclic_permutation(boost::begin(range1), boost::end(range1),
                                 boost::forward<Range2>(range2));
}
} // end namespace cyclic_permutation_detail

namespace core {
    using cyclic_permutation_detail::is_cyclic_permutation;
}
} // end namespace d2

#endif // !D2_CORE_CYCLIC_PERMUTATION_HPP
