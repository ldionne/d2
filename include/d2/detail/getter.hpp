/**
 * This file defines the `getter` functor.
 */

#ifndef D2_DETAIL_GETTER_HPP
#define D2_DETAIL_GETTER_HPP

namespace d2 {
namespace detail {

/**
 * Functor fetching a member inside an object.
 */
template <typename Type, typename Tag>
class getter {
    typedef Type Tag::* Ptr;
    Ptr ptr_;

public:
    explicit getter(Ptr ptr) : ptr_(ptr) { }

    template <typename Sig>
    struct result;

    template <typename This>
    struct result<This(Tag&)> {
        typedef Type& type;
    };

    template <typename This>
    struct result<This(Tag const&)> {
        typedef Type const& type;
    };

    typename result<getter(Tag&)>::type operator()(Tag& t) const {
        return t.*ptr_;
    }

    typename result<getter(Tag const&)>::type operator()(Tag const& t) const {
        return t.*ptr_;
    }
};

/**
 * `getter` of a member specified at compile time.
 */
template <typename Type, typename Tag, Type Tag::* ptr>
struct fixed_getter : getter<Type, Tag> {
    fixed_getter() : getter<Type, Tag>(ptr) { }
};

/**
 * Helper function to create a `getter`.
 */
template <typename Type, typename Tag>
getter<Type, Tag> get(Type Tag::*ptr) {
    return getter<Type, Tag>(ptr);
}

} // end namespace detail
} // end namespace d2

#endif // !D2_DETAIL_GETTER_HPP
