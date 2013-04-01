/*!
 * @file
 * This file contains unit tests for the wrappers for the `Lockable` concept.
 */

#include <d2/lockable.hpp>
#include <d2/detail/thread_lockable_archetypes.hpp>

#include <boost/concept/assert.hpp>
#include <boost/static_assert.hpp>
#include <boost/thread/lockable_concepts.hpp>
#include <boost/thread/lockable_traits.hpp>


namespace {
// non recursive
BOOST_CONCEPT_ASSERT((
    boost::Lockable<
        d2::lockable<d2::detail::lockable_archetype<> >
    >
));

BOOST_STATIC_ASSERT((
    ::boost::sync::is_lockable<
        d2::lockable<d2::detail::lockable_archetype<> >
    >::value
));

BOOST_STATIC_ASSERT((
    !::boost::sync::is_recursive_lockable<
        d2::lockable<d2::detail::lockable_archetype<> >
    >::value
));


// recursive
BOOST_CONCEPT_ASSERT((
    boost::Lockable<
        d2::recursive_lockable<d2::detail::lockable_archetype<> >
    >
));

BOOST_STATIC_ASSERT((
    ::boost::sync::is_lockable<
        d2::recursive_lockable<d2::detail::lockable_archetype<> >
    >::value
));

BOOST_STATIC_ASSERT((
    ::boost::sync::is_recursive_lockable<
        d2::recursive_lockable<d2::detail::lockable_archetype<> >
    >::value
));


class lockable : public d2::lockable_mixin<lockable> {
    friend class d2::lockable_mixin<lockable>;
    void lock_impl() { }
    void unlock_impl() { }
    bool try_lock_impl() { return false; }
};
BOOST_CONCEPT_ASSERT((boost::Lockable<lockable>));

BOOST_STATIC_ASSERT((::boost::sync::is_lockable<lockable>::value));

BOOST_STATIC_ASSERT((
    !::boost::sync::is_recursive_lockable<lockable>::value
));


class recursive_lockable
    : public d2::recursive_lockable_mixin<recursive_lockable>
{
    friend class d2::recursive_lockable_mixin<recursive_lockable>;
    void lock_impl() { }
    void unlock_impl() { }
    bool try_lock_impl() { return false; }
};
BOOST_CONCEPT_ASSERT((boost::Lockable<recursive_lockable>));

BOOST_STATIC_ASSERT((::boost::sync::is_lockable<recursive_lockable>::value));

// with the mixin, we can't specialize this trait automatically. the derived
// class will have to do it.
BOOST_STATIC_ASSERT((
    !::boost::sync::is_recursive_lockable<recursive_lockable>::value
));
} // end anonymous namespace


int main() {

}
