/**
 * This file contains unit tests for the `pair_adjacent` metafunction.
 */

#include <d2/detail/pair_adjacent.hpp>

#include <boost/mpl/assert.hpp>
#include <boost/mpl/equal.hpp>
#include <boost/mpl/pair.hpp>
#include <boost/mpl/vector.hpp>


namespace d2 {
namespace test {

template <typename Input, typename Expected>
struct make_test {
    typedef typename detail::pair_adjacent<Input>::type Paired;
    typedef typename boost::mpl::equal<Expected, Paired>::type type;
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
using boost::mpl::pair;

BOOST_MPL_ASSERT((make_test<
    vector<A, B,
           C, D,
           E, F>,
    vector<
        pair<A, B>,
        pair<C, D>,
        pair<E, F>
    >
>));

BOOST_MPL_ASSERT((make_test<
    vector<A, B>,
    vector<pair<A, B> >
>));

BOOST_MPL_ASSERT((make_test<
    vector<>,
    vector<>
>));

int main() {

}
