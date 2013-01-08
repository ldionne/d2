/**
 * This file defines the `container_view` class.
 */

#ifndef D2_SANDBOX_CONTAINER_VIEW_HPP
#define D2_SANDBOX_CONTAINER_VIEW_HPP

#include <d2/sandbox/basic_container.hpp>

#include <algorithm>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/mpl/and.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/has_xxx.hpp>
#include <boost/operators.hpp>
#include <boost/type_traits/remove_reference.hpp>


namespace d2 {

namespace sandbox {
namespace detail {
BOOST_MPL_HAS_XXX_TRAIT_DEF(mapped_type)
BOOST_MPL_HAS_XXX_TRAIT_DEF(key_type)

template <typename Container>
struct is_probably_a_map
    : boost::mpl::and_<
        has_mapped_type<Container>,
        has_key_type<Container>
    >
{ };

template <typename Container, typename IsMap>
struct provide_map_typedefs {
protected:
    typedef typename Container::reference mapped_or_value_type;
};

template <typename Container>
struct provide_map_typedefs<Container, boost::mpl::true_> {
    typedef typename Container::mapped_type mapped_type;
    typedef typename Container::key_type key_type;

protected:
    typedef mapped_type mapped_or_value_type;
};

template <typename Container>
struct provide_container_typedefs
    : provide_map_typedefs<
        Container,
        typename is_probably_a_map<Container>::type
    >
{
    typedef typename Container::value_type value_type;
    typedef typename Container::allocator_type allocator_type;
    typedef typename Container::size_type size_type;
    typedef typename Container::difference_type difference_type;
    typedef typename Container::reference reference;
    typedef typename Container::const_reference const_reference;
    typedef typename Container::pointer pointer;
    typedef typename Container::const_pointer const_pointer;
    typedef typename Container::iterator iterator;
    typedef typename Container::const_iterator const_iterator;
};
} // end namespace detail

template <typename Container, typename Accessor>
struct container_view
    : detail::provide_container_typedefs<Container>,
      boost::equality_comparable<container_view<Container, Accessor> >,
      basic_container<
        container_view<Container, Accessor>,
        boost::transform_iterator<
            Accessor,
            typename detail::provide_container_typedefs<Container>::iterator
        >,
        boost::transform_iterator<
            Accessor,
            typename detail::provide_container_typedefs<Container>::
                                                                const_iterator
        >
    >
{
    typedef boost::transform_iterator<
        Accessor,
        typename detail::provide_container_typedefs<Container>::iterator
    > iterator;

    typedef boost::transform_iterator<
        Accessor,
        typename detail::provide_container_typedefs<Container>::const_iterator
    > const_iterator;

    explicit container_view(Container& container) : self_(container) { }

    template <typename OtherContainer>
    operator OtherContainer() const {
        return convert_to<OtherContainer>();
    }

    typename container_view::size_type size() const {
        return self_.size();
    }

    template <typename Key>
    typename container_view::mapped_or_value_type& operator[](Key const& key) {
        return self_[key];
    }

    template <typename Other>
    friend bool operator==(container_view const& self, Other const& other) {
        return self.size() == other.size() &&
               std::equal(self.begin(), self.end(), other.begin());
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

template <typename Container>
class item_view {
    struct identity_accessor {
        template <typename Sig> struct result;

        template <typename This, typename T>
        struct result<This(T&)>
        { typedef T& type; };

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

public:
    typedef container_view<Container, identity_accessor> type;
};

template <typename Container>
class key_view {
    struct first_accessor {
        template <typename Sig> struct result;

        template <typename This, typename T>
        struct result<This(T&)>
        { typedef typename T::first_type& type; };

        template <typename This, typename T>
        struct result<This(T const&)>
        { typedef typename T::first_type const& type; };

        template <typename T>
        typename result<first_accessor(T&)>::type operator()(T& t) const {
            return t.first;
        }

        template <typename T>
        typename result<first_accessor(T const&)>::type
        operator()(T const& t) const {
            return t.first;
        }
    };

public:
    typedef container_view<Container, first_accessor> type;
};

template <typename Container>
class value_view {
    struct second_accessor {
        template <typename Sig> struct result;

        template <typename This, typename T>
        struct result<This(T&)>
        { typedef typename T::second_type& type; };

        template <typename This, typename T>
        struct result<This(T const&)>
        { typedef typename T::second_type const& type; };

        template <typename T>
        typename result<second_accessor(T&)>::type operator()(T& t) const {
            return t.second;
        }

        template <typename T>
        typename result<second_accessor(T const&)>::type
        operator()(T const& t) const {
            return t.second;
        }
    };

public:
    typedef container_view<Container, second_accessor> type;
};

} // end namespace sandbox
} // end namespace d2

#endif // !D2_SANDBOX_CONTAINER_VIEW_HPP
