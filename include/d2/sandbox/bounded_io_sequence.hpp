/**
 * This file defines bounded input/ouput sequences.
 */

#ifndef D2_SANDBOX_BOUNDED_IO_SEQUENCE_HPP
#define D2_SANDBOX_BOUNDED_IO_SEQUENCE_HPP

#include <algorithm>
#include <boost/algorithm/cxx11/copy_n.hpp>
#include <boost/assert.hpp>
#include <boost/concept_check.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/iterator/iterator_traits.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/type_traits/has_dereference.hpp>
#include <boost/type_traits/has_post_increment.hpp>
#include <boost/type_traits/has_pre_increment.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/utility/enable_if.hpp>
#include <cstddef>
#include <iterator>


namespace d2 {
namespace sandbox {

template <typename Iterator,
          typename OutputType = typename boost::iterator_value<Iterator>::type>
class bounded_output_sequence {
    Iterator first_, last_;

public:
    template <typename Container>
    explicit bounded_output_sequence(Container const& c)
        : first_(boost::begin(c)), last_(boost::end(c))
    { }

    bounded_output_sequence(Iterator first, Iterator last)
        : first_(first), last_(last)
    { }

    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os,
                               bounded_output_sequence const& self) {
        // The iterators must model at least ForwardIterator because we
        // compute their distance.
        BOOST_CONCEPT_ASSERT((boost::ForwardIterator<Iterator>));
        os << std::distance(self.first_, self.last_) << ':';
        std::copy(self.first_, self.last_,
                  std::ostream_iterator<OutputType>(os));
        return os;
    }
};

template <typename Iterator>
bounded_output_sequence<Iterator>
make_bounded_output_sequence(Iterator first, Iterator last) {
    return bounded_output_sequence<Iterator>(first, last);
}

template <typename Container>
bounded_output_sequence<typename Container::const_iterator>
make_bounded_output_sequence(Container const& c) {
    return bounded_output_sequence<typename Container::const_iterator>(c);
}


template <typename Iterator,
          typename InputType = typename boost::iterator_value<Iterator>::type>
class bounded_input_sequence {
    Iterator out_;

public:
    template <typename Container>
    explicit bounded_input_sequence(Container& c,
        typename boost::enable_if<
                boost::is_same<Iterator,std::back_insert_iterator<Container> >
            >::type* =0)
        : out_(c)
    { }

    explicit bounded_input_sequence(Iterator out) : out_(out) { }

    template <typename Istream>
    friend Istream& operator>>(Istream& is, bounded_input_sequence self) {
        BOOST_CONCEPT_ASSERT((boost::OutputIterator<Iterator, InputType>));
        std::size_t size;
        char colon;
        is >> size >> colon;
        std::istream_iterator<InputType> it(is);
        boost::algorithm::copy_n(it, size, self.out_);
        return is;
    }
};

template <typename Iterator>
struct is_iterator
    : boost::mpl::and_<
        boost::has_dereference<Iterator>,
        boost::has_pre_increment<Iterator>,
        boost::has_post_increment<Iterator>
    >
{ };

template <typename InputType, typename Iterator>
typename boost::enable_if<is_iterator<Iterator>,
bounded_input_sequence<Iterator, InputType> >::type
make_bounded_input_sequence(Iterator out) {
    return bounded_input_sequence<Iterator, InputType>(out);
}

template <typename Container>
typename boost::disable_if<is_iterator<Container>,
bounded_input_sequence<
    std::back_insert_iterator<Container>, typename Container::value_type
> >::type
make_bounded_input_sequence(Container& c) {
    return bounded_input_sequence<
            std::back_insert_iterator<Container>,
            typename Container::value_type
        >(c);
}


template <typename Iterator,
          typename IOType = typename boost::iterator_value<Iterator>::type>
class bounded_io_sequence {
    Iterator first_, last_;

    struct dont_go_past : boost::iterator_adaptor<dont_go_past, Iterator,
                            boost::use_default, std::output_iterator_tag> {

        explicit dont_go_past(Iterator first, Iterator last)
            : dont_go_past::iterator_adaptor_(first), boundary_(last)
        { }

    private:
        friend class boost::iterator_core_access;

        void increment() {
            BOOST_ASSERT_MSG(this->base_reference() != boundary_,
                        "incrementing an iterator past the end of the range");
            ++this->base_reference();
        }

        Iterator boundary_;
    };

public:
    template <typename Container>
    explicit bounded_io_sequence(Container& c)
        : first_(boost::begin(c)), last_(boost::end(c))
    { }

    bounded_io_sequence(Iterator first, Iterator last)
        : first_(first), last_(last)
    { }

    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os, bounded_io_sequence const& self) {
        return os << make_bounded_output_sequence(self.first_, self.last_);
    }

    template <typename Istream>
    friend Istream& operator>>(Istream& is, bounded_io_sequence self) {
        return is >> bounded_input_sequence<dont_go_past, IOType>(
                                    dont_go_past(self.first_, self.last_));
    }
};

template <typename Iterator>
bounded_io_sequence<Iterator>
make_bounded_io_sequence(Iterator first, Iterator last) {
    return bounded_io_sequence<Iterator>(first, last);
}

template <typename Container>
bounded_io_sequence<typename Container::iterator>
make_bounded_io_sequence(Container& c) {
    return bounded_io_sequence<typename Container::iterator>(c);
}


template <typename T>
class bounded_for_io {
    T value_;

public:
    bounded_for_io() { }

    // This is implicit because it is only a thin wrapper around T.
    bounded_for_io(T const& t) : value_(t) { }

    operator T const&() const {
        return value_;
    }

    operator T&() {
        return value_;
    }

    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os, bounded_for_io const& self) {
        return os << make_bounded_output_sequence(self.value_);
    }

    template <typename Istream>
    friend Istream& operator>>(Istream& is, bounded_for_io& self) {
        return is >> make_bounded_input_sequence(self.value_);
    }
};

} // end namespace sandbox
} // end namespace d2

#endif // !D2_SANDBOX_BOUNDED_IO_SEQUENCE_HPP
