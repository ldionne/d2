/**
 * This file defines an interface to manipulate a call stack.
 */

#ifndef D2_BTRACE_CALL_STACK_HPP
#define D2_BTRACE_CALL_STACK_HPP

#include <d2/btrace/detail/basic_container.hpp>
#include <d2/btrace/detail/call_stack.hpp>

#include <boost/array.hpp>
#include <boost/assert.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/iterator/iterator_categories.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/static_assert.hpp>
#include <cstddef>
#include <iterator>
#include <string>
#include <vector>


namespace d2 {
namespace btrace {

struct stack_frame {
    void* address;
    std::size_t depth;

    std::string str() const {
        return detail::get_symbol_at(address);
    }

    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os, stack_frame const& f) {
        return os << f.str(), os;
    }
};

namespace detail {

template <typename Iterator>
class stack_frame_iterator
    : public boost::iterator_adaptor<
                stack_frame_iterator<Iterator>, Iterator,
                stack_frame, boost::bidirectional_traversal_tag, stack_frame>{

    friend class boost::iterator_core_access;
    typedef typename std::iterator_traits<Iterator>::difference_type Depth;
    Depth depth_;

    stack_frame dereference() const {
        stack_frame frame;
        frame.address = *this->base_reference();
        frame.depth = depth_;
        return frame;
    }

    void increment() {
        ++this->base_reference();
        ++depth_;
    }

    void decrement() {
        --this->base_reference();
        --depth_;
    }

    bool equal(stack_frame_iterator const& other) const {
        return this->base_reference() == other.base_reference() &&
               depth_ == other.depth_;
    }

public:
    explicit stack_frame_iterator(Depth depth, Iterator it)
        : stack_frame_iterator::iterator_adaptor_(it), depth_(depth)
    { }

public:
    stack_frame_iterator() : depth_(0) { }
};

template <typename Derived, typename Frames>
struct call_stack_base : basic_container<Derived,
                    stack_frame_iterator<typename Frames::iterator>,
                    stack_frame_iterator<typename Frames::const_iterator> > {
    typedef stack_frame value_type;
    typedef value_type reference;
    typedef value_type const_reference;
    typedef typename Frames::size_type size_type;
    typedef typename Frames::difference_type difference_type;
    typedef stack_frame_iterator<typename Frames::iterator> iterator;
    typedef stack_frame_iterator<typename Frames::const_iterator>
                                                            const_iterator;
};

} // end namespace detail


/**
 * Call stack with a depth limit determined at compile-time.
 */
template <std::size_t N>
class call_stack : public detail::call_stack_base<
                                    call_stack<N>, boost::array<void*, N> > {
    BOOST_STATIC_ASSERT_MSG(N > 0, "maximum size of call_stack must be "
                                   "greater than 0");
    typedef boost::array<void*, N> Frames;
    Frames frames_;
    typename Frames::size_type n_;

public:
    typedef typename call_stack::iterator iterator;
    typedef typename call_stack::const_iterator const_iterator;

    call_stack() : n_(0) {
        void** it = detail::copy_frame_pointers(N, &frames_[0]);
        n_ = std::distance(boost::begin(frames_), it);
    }

    typename call_stack::size_type size() const {
        return n_;
    }

private:
    friend class detail::basic_container_core_access;
    iterator iterator_begin() {
        return iterator(0, boost::begin(frames_));
    }

    const_iterator const_iterator_begin() const {
        return const_iterator(0, boost::begin(frames_));
    }

    iterator iterator_end() {
        return iterator(n_, boost::begin(frames_) + n_);
    }

    const_iterator const_iterator_end() const {
        return const_iterator(n_, boost::begin(frames_) + n_);
    }
};


/**
 * Call stack with a depth limit determined at runtime.
 */
struct dynamic_call_stack : detail::call_stack_base<
                                    dynamic_call_stack, std::vector<void*> > {
    inline explicit dynamic_call_stack(size_type max_frames) {
        frames_.reserve(max_frames);
        detail::copy_frame_pointers(max_frames, std::back_inserter(frames_));
        BOOST_ASSERT_MSG(!frames_.empty(),
                    "the backtrace buffer of a call_stack may not be empty");
    }

    dynamic_call_stack::size_type size() const {
        return frames_.size();
    }

private:
    friend class detail::basic_container_core_access;

    inline iterator iterator_begin() {
        return iterator(0, boost::begin(frames_));
    }

    inline const_iterator const_iterator_begin() const {
        return const_iterator(0, boost::begin(frames_));
    }

    inline iterator iterator_end() {
        return iterator(frames_.size(), boost::end(frames_));
    }

    inline const_iterator const_iterator_end() const {
        return const_iterator(frames_.size(), boost::end(frames_));
    }

    std::vector<void*> frames_;
};

} // end namespace btrace
} // end namespace d2

#endif // !D2_BTRACE_CALL_STACK_HPP
