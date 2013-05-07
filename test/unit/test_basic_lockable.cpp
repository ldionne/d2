/*!
 * @file
 * This file contains unit tests for the wrappers for the `BasicLockable`
 * concept.
 */

#include <d2/basic_lockable.hpp>
#include <d2/detail/thread_lockable_archetypes.hpp>

#include <boost/concept/assert.hpp>
#include <boost/static_assert.hpp>
#include <boost/thread/lockable_concepts.hpp>
#include <boost/thread/lockable_traits.hpp>


namespace {
// non recursive
BOOST_CONCEPT_ASSERT((
    boost::BasicLockable<
        d2::basic_lockable<d2::detail::basic_lockable_archetype<> >
    >
));

BOOST_STATIC_ASSERT((
    ::boost::sync::is_basic_lockable<
        d2::basic_lockable<d2::detail::basic_lockable_archetype<> >
    >::value
));

BOOST_STATIC_ASSERT((
    !::boost::sync::is_recursive_lockable<
        d2::basic_lockable<d2::detail::basic_lockable_archetype<> >
    >::value
));


// recursive
BOOST_CONCEPT_ASSERT((
    boost::BasicLockable<
        d2::recursive_basic_lockable<d2::detail::basic_lockable_archetype<> >
    >
));

BOOST_STATIC_ASSERT((
    ::boost::sync::is_basic_lockable<
        d2::recursive_basic_lockable<d2::detail::basic_lockable_archetype<> >
    >::value
));

BOOST_STATIC_ASSERT((
    ::boost::sync::is_recursive_basic_lockable<
        d2::recursive_basic_lockable<d2::detail::basic_lockable_archetype<> >
    >::value
));


class basic_lockable : public d2::basic_lockable_mixin<basic_lockable> {
    friend class d2::access;
    void lock_impl() { }
    void unlock_impl() { }
};
BOOST_CONCEPT_ASSERT((boost::BasicLockable<basic_lockable>));

BOOST_STATIC_ASSERT((::boost::sync::is_basic_lockable<basic_lockable>::value));

BOOST_STATIC_ASSERT((
    !::boost::sync::is_recursive_lockable<basic_lockable>::value
));


class recursive_basic_lockable
    : public d2::recursive_basic_lockable_mixin<recursive_basic_lockable>
{
    friend class d2::access;
    void lock_impl() { }
    void unlock_impl() { }
};
BOOST_CONCEPT_ASSERT((boost::BasicLockable<recursive_basic_lockable>));

BOOST_STATIC_ASSERT((
    ::boost::sync::is_basic_lockable<recursive_basic_lockable>::value
));

BOOST_STATIC_ASSERT((
    !::boost::sync::is_recursive_basic_lockable<
        recursive_basic_lockable
    >::value
));
} // end anonymous namespace


int main() {

}
