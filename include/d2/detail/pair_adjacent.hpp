/**
 * This file defines the `pair_adjacent` metafunction.
 */

#ifndef D2_DETAIL_PAIR_ADJACENT_HPP
#define D2_DETAIL_PAIR_ADJACENT_HPP

#include <boost/mpl/apply.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/begin.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/deref.hpp>
#include <boost/mpl/end.hpp>
#include <boost/mpl/next.hpp>
#include <boost/mpl/pair.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/vector.hpp>


namespace d2 {
namespace detail {

namespace pair_adjacent_detail {
    template <template <typename T1, typename T2> class Pair>
    struct make {
        template <typename T1, typename T2>
        struct apply {
            typedef Pair<T1, T2> type;
        };
    };
}

/**
 * Metafunction returning a sequence of pairs of the elements that were
 * adjacent in the original sequence.
 */
template <typename Sequence,
          typename MakePair = pair_adjacent_detail::make<boost::mpl::pair> >
class pair_adjacent {
    template <typename T>
    struct is_even
        : boost::mpl::bool_<T::type::value % 2 == 0>
    { };

    template <typename First, typename Last, typename OutSequence>
    struct populate {
        typedef typename boost::mpl::deref<First>::type Key;

        typedef typename boost::mpl::next<First>::type Next;
        typedef typename boost::mpl::deref<Next>::type Value;

        typedef typename boost::mpl::push_back<
                    OutSequence,
                    typename boost::mpl::apply<MakePair, Key, Value>::type
                >::type NewOutSequence;

        typedef typename populate<
                    typename boost::mpl::next<Next>::type,
                    Last,
                    NewOutSequence
                >::type type;
    };

    template <typename Last, typename OutSequence>
    struct populate<Last, Last, OutSequence> {
        typedef OutSequence type;
    };

public:
    // We can never group the sequence in pairs if there is not an even
    // number of elements.
    BOOST_MPL_ASSERT((is_even<typename boost::mpl::size<Sequence>::type>));

    typedef typename populate<
                typename boost::mpl::begin<Sequence>::type,
                typename boost::mpl::end<Sequence>::type,
                boost::mpl::vector<>
            >::type type;
};

} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_PAIR_ADJACENT_HPP
