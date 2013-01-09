/**
 * This file defines the `container_view` class.
 */

#ifndef D2_SANDBOX_CONTAINER_VIEW_HPP
#define D2_SANDBOX_CONTAINER_VIEW_HPP

#include <d2/sandbox/basic_container.hpp>

#include <algorithm>
#include <boost/iterator/iterator_traits.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/operators.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/utility/result_of.hpp>
#include <iterator>


namespace d2 {

namespace sandbox {
template <typename Container, typename Accessor>
struct container_view
    : boost::equality_comparable<container_view<Container, Accessor> >,
      basic_container<
        container_view<Container, Accessor>,
        boost::transform_iterator<
            Accessor, typename Container::iterator
        >,
        boost::transform_iterator<
            Accessor, typename Container::const_iterator
        >
    >
{
    using typename container_view::basic_container_::iterator;
    using typename container_view::basic_container_::const_iterator;

    typedef typename boost::result_of<
                Accessor(typename Container::value_type)
            >::type value_type;

    typedef typename boost::result_of<
                Accessor(typename Container::reference)
            >::type reference;

    typedef typename boost::result_of<
                Accessor(typename Container::const_reference)
            >::type const_reference;

    typedef void pointer;
    typedef void const_pointer;
    typedef typename Container::size_type size_type;

    explicit container_view(Container& container)
        : self_(container)
    { }

    template <typename OtherContainer>
    operator OtherContainer() const {
        return convert_to<OtherContainer>();
    }

    size_type size() const {
        return self_.size();
    }

    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os, container_view const& self) {
        typedef std::ostream_iterator<value_type> OstreamIterator;
        std::copy(self.begin(), self.end(), OstreamIterator(os, ", "));
        return os;
    }

private:
    friend class basic_container_core_access;

    iterator iterator_begin() {
        return iterator(self_.begin());
    }

    const_iterator const_iterator_begin() const {
        return const_iterator(self_.begin());
    }

    iterator iterator_end() {
        return iterator(self_.end());
    }

    const_iterator const_iterator_end() const {
        return const_iterator(self_.end());
    }

    template <typename OtherContainer>
    OtherContainer convert_to() const {
        return OtherContainer(this->begin(), this->end());
    }

    Container& self_;
};

struct identity_accessor {
    template <typename Sig> struct result;

    template <typename This, typename T>
    struct result<This(T)>
    { typedef T type; };

    template <typename T>
    typename result<identity_accessor(T&)>::type operator()(T& t) const {
        return t;
    }

    template <typename T>
    typename result<identity_accessor(T const&)>::type
    operator()(T const& t) const {
        return t;
    }
};

template <typename T, T Ptr, typename NextAccessor = identity_accessor>
struct member_accessor;

template <typename Tag, typename Type, Type Tag::* Ptr, typename NextAccessor>
struct member_accessor<Type Tag::*, Ptr, NextAccessor> {
    template <typename Sig> struct result;

    template <typename This>
    struct result<This(Tag&)>
        : boost::result_of<NextAccessor(Type&)>
    { };

    template <typename This>
    struct result<This(Tag const&)>
        : boost::result_of<NextAccessor(Type const&)>
    { };

    template <typename This>
    struct result<This(Tag)>
        : boost::result_of<NextAccessor(Type)>
    { };

    template <typename T>
    typename result<member_accessor(T&)>::type operator()(T& t) const {
        return NextAccessor()(t.*Ptr);
    }

    template <typename T>
    typename result<member_accessor(T const&)>::type
    operator()(T const& t) const {
        return NextAccessor()(t.*Ptr);
    }
};

template <template <typename T> class Unbound,
          typename NextAccessor = identity_accessor>
struct rebind_accessor {
    template <typename Sig> struct result;

    template <typename This, typename T>
    struct result<This(T)>
        : boost::result_of<
            NextAccessor(
                typename boost::result_of<
                    Unbound<typename boost::remove_reference<T>::type>(T)
                >::type
            )
        >
    { };

    template <typename T>
    typename result<rebind_accessor(T&)>::type
    operator()(T& object) const {
        return Unbound<T>()(object);
    }

    template <typename T>
    typename result<rebind_accessor(T const&)>::type
    operator()(T const& object) const {
        return Unbound<T>()(object);
    }
};

namespace detail {
    template <typename Pair>
    struct first_accessor_helper
        : member_accessor<
            typename Pair::first_type Pair::*,
            &Pair::first
        >
    { };

    template <typename Pair>
    struct second_accessor_helper
        : member_accessor<
            typename Pair::second_type Pair::*,
            &Pair::second
        >
    { };
} // end namespace detail

template <typename NextAccessor = identity_accessor>
struct second_accessor
    : rebind_accessor<
        detail::second_accessor_helper, NextAccessor
    >
{ };

template <typename NextAccessor = identity_accessor>
struct first_accessor
    : rebind_accessor<
        detail::first_accessor_helper, NextAccessor
    >
{ };

} // end namespace sandbox
} // end namespace d2

#endif // !D2_SANDBOX_CONTAINER_VIEW_HPP
