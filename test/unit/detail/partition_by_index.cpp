/**
 * This file contains unit tests for the `partition_by_index` metafunction.
 */

#include <d2/detail/partition_by_index.hpp>

#include <boost/mpl/and.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/equal.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/vector.hpp>


namespace d2 {
namespace test {

template <typename T>
struct is_even
    : boost::mpl::bool_<T::value % 2 == 0>
{ };

template <typename T>
struct is_odd
    : boost::mpl::not_<is_even<T> >
{ };

template <typename Sequence, typename Pred, typename Left, typename Right>
struct make_test {
    typedef typename detail::partition_by_index<Sequence, Pred>::type Seq;
    typedef typename Seq::first ActualLeft;
    typedef typename Seq::second ActualRight;

    typedef typename boost::mpl::equal<ActualLeft, Left>::type LeftIsOk;
    typedef typename boost::mpl::equal<ActualRight, Right>::type RightIsOk;
    typedef typename boost::mpl::and_<LeftIsOk, RightIsOk>::type type;
};

struct A { };
struct B { };
struct C { };
struct D { };
struct E { };
struct F { };

} // end namespace test
} // end namespace d2


using namespace d2::test;
using boost::mpl::vector;
typedef boost::mpl::_1 arg1;

BOOST_MPL_ASSERT((make_test<
    vector<A, B, C, D, E, F>,
    is_even<arg1>,
    vector<A, C, E>, vector<B, D, F>
>));

BOOST_MPL_ASSERT((make_test<
    vector<A, B, C, D, E, F>,
    is_odd<arg1>,
    vector<B, D, F>, vector<A, C, E>
>));

BOOST_MPL_ASSERT((make_test<
    vector<A, B, C, D, E>,
    is_even<arg1>,
    vector<A, C, E>, vector<B, D>
>));

BOOST_MPL_ASSERT((make_test<
    vector<>,
    is_even<arg1>,
    vector<>, vector<>
>));

int main() {

}
