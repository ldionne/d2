/**
 * This file contains compile-time tests for event traits.
 */

#include <d2/event_traits.hpp>

#include <boost/mpl/assert.hpp>
#include <boost/type_traits/is_same.hpp>


namespace {

struct ValidEvent {
    typedef d2::thread_scope event_scope;
    typedef d2::strict_order_policy ordering_policy;

    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os, ValidEvent const& self);
};

BOOST_MPL_ASSERT((boost::is_same<
    d2::thread_scope,
    d2::event_scope<ValidEvent>::type
>));

BOOST_MPL_ASSERT((boost::is_same<
    d2::strict_order_policy,
    d2::ordering_policy<ValidEvent>::type
>));

BOOST_MPL_ASSERT((d2::is_event<ValidEvent>));

BOOST_MPL_ASSERT((d2::has_event_scope<ValidEvent, d2::thread_scope>));
BOOST_MPL_ASSERT_NOT((d2::has_event_scope<ValidEvent, d2::machine_scope>));

BOOST_MPL_ASSERT((
    d2::has_ordering_policy<ValidEvent, d2::strict_order_policy>
));
BOOST_MPL_ASSERT_NOT((
    d2::has_ordering_policy<ValidEvent, d2::no_order_policy>
));


struct MissingEventScope {
    typedef d2::strict_order_policy ordering_policy;

    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os, ValidEvent const& self);
};

BOOST_MPL_ASSERT((boost::is_same<
    d2::strict_order_policy,
    d2::ordering_policy<MissingEventScope>::type
>));

BOOST_MPL_ASSERT_NOT((
    d2::is_event<MissingEventScope>
));

BOOST_MPL_ASSERT_NOT((
    d2::has_event_scope<MissingEventScope, d2::thread_scope>
));

BOOST_MPL_ASSERT((
    d2::has_ordering_policy<MissingEventScope, d2::strict_order_policy>
));

} // end anonymous namespace

int main() {

}
