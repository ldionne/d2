/**
 * This file defines the `partition_by_index` metafunction.
 */

#ifndef D2_DETAIL_PARTITION_BY_INDEX_HPP
#define D2_DETAIL_PARTITION_BY_INDEX_HPP

#include <boost/mpl/apply.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/back_inserter.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/pair.hpp>
#include <boost/mpl/partition.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/range_c.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/zip_view.hpp>


namespace d2 {
namespace detail {

/**
 * Metafunction partitionning a sequence by applying a predicate on its
 * indices.
 */
template <typename Sequence, typename Predicate>
class partition_by_index {
    typedef typename boost::mpl::range_c<
                typename boost::mpl::size<Sequence>::value_type,
                0, ::boost::mpl::size<Sequence>::value
            > indices;

    typedef typename boost::mpl::zip_view<
                        boost::mpl::vector<indices, Sequence>
                    >::type indexed_sequence;

    struct apply_pred_to_index {
        template <typename Zipped>
        struct apply
            : boost::mpl::apply<
                Predicate, typename boost::mpl::at_c<Zipped, 0>::type
            >
        { };
    };

    struct get_element {
        template <typename Zipped>
        struct apply
            : boost::mpl::at_c<Zipped, 1>
        { };
    };

    typedef typename boost::mpl::partition<
                indexed_sequence,
                apply_pred_to_index,
                boost::mpl::back_inserter<boost::mpl::vector<> >,
                boost::mpl::back_inserter<boost::mpl::vector<> >
            >::type partitioned;

    typedef typename boost::mpl::transform<
                typename boost::mpl::first<partitioned>::type,
                get_element
            >::type left;

    typedef typename boost::mpl::transform<
                typename boost::mpl::second<partitioned>::type,
                get_element
            >::type right;

public:
    typedef boost::mpl::pair<left, right> type;
};

} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_PARTITION_BY_INDEX_HPP
