/**
 * This file defines a mixin class to implement the different flavors of
 * begin and end functions for containers.
 */

#ifndef D2_DETAIL_BASIC_CONTAINER_HPP
#define D2_DETAIL_BASIC_CONTAINER_HPP

namespace d2 {
namespace basic_container_detail {
class basic_container_core_access {
public:
    template <typename Facade>
    static typename Facade::iterator iterator_begin(Facade& f) {
        return f.iterator_begin();
    }

    template <typename Facade>
    static typename Facade::const_iterator
    const_iterator_begin(Facade const& f) {
        return f.const_iterator_begin();
    }

    template <typename Facade>
    static typename Facade::iterator iterator_end(Facade& f) {
        return f.iterator_end();
    }

    template <typename Facade>
    static typename Facade::const_iterator const_iterator_end(Facade const&f){
        return f.const_iterator_end();
    }
};

/**
 * Implements all the different `begin()` and `end()` versions. The `Derived`
 * class must implement `iterator_begin()`, `const_iterator_begin()`,
 * `iterator_end()`, and `const_iterator_end()`. It must also befriend
 * the `basic_container_core_access` class.
 */
template <typename Derived, typename Iterator, typename ConstIterator>
class basic_container {
    Derived& derived() {
        return *static_cast<Derived*>(this);
    }

    Derived const& derived() const {
        return *static_cast<Derived const*>(this);
    }

public:
    typedef Iterator iterator;
    typedef ConstIterator const_iterator;

    iterator begin() {
        return basic_container_core_access::iterator_begin(derived());
    }

    const_iterator begin() const {
        return basic_container_core_access::const_iterator_begin(derived());
    }

    const_iterator cbegin() const {
        return basic_container_core_access::const_iterator_begin(derived());
    }

    friend iterator begin(Derived& d) {
        return basic_container_core_access::iterator_begin(d.derived());
    }

    friend const_iterator begin(Derived const& d) {
        return basic_container_core_access::const_iterator_begin(d.derived());
    }


    iterator end() {
        return basic_container_core_access::iterator_end(derived());
    }

    const_iterator end() const {
        return basic_container_core_access::const_iterator_end(derived());
    }

    const_iterator cend() const {
        return basic_container_core_access::const_iterator_end(derived());
    }

    friend iterator end(Derived& d) {
        return basic_container_core_access::iterator_end(d.derived());
    }

    friend const_iterator end(Derived const& d) {
        return basic_container_core_access::const_iterator_end(d.derived());
    }

protected:
    typedef basic_container basic_container_;

};
} // end namespace basic_container_detail

namespace detail {
    using basic_container_detail::basic_container;
    using basic_container_detail::basic_container_core_access;
}
} // end namespace d2

#endif // !D2_DETAIL_BASIC_CONTAINER_HPP
