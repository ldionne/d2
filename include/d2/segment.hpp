/**
 * This file defines the `Segment` class.
 */

#ifndef D2_SEGMENT_HPP
#define D2_SEGMENT_HPP

#include <boost/operators.hpp>
#include <cstddef>


namespace d2 {

/**
 * Represents a segment inside which operations happen in a serialized
 * manner inside a program.
 */
struct Segment : boost::totally_ordered<Segment,
                 boost::additive<Segment,
                 boost::unit_steppable<Segment> > > {

    inline Segment() : value_(0) { }

    friend bool operator==(Segment const& a, Segment const& b) {
        return a.value_ == b.value_;
    }

    friend bool operator<(Segment const& a, Segment const& b) {
        return a.value_ < b.value_;
    }

    friend bool operator>(Segment const& a, Segment const& b) {
        return a.value_ > b.value_;
    }

    friend Segment& operator+=(Segment& a, Segment const& b) {
        a.value_ += b.value_;
        return a;
    }

    friend Segment& operator-=(Segment& a, Segment const& b) {
        a.value_ -= b.value_;
        return a;
    }

    inline Segment& operator++() {
        ++value_;
        return *this;
    }

    inline Segment& operator--() {
        --value_;
        return *this;
    }

    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os, Segment const& self) {
        return os << self.value_, os;
    }

    template <typename Istream>
    friend Istream& operator>>(Istream& is, Segment& self) {
        return is >> self.value_, is;
    }

    friend std::size_t hash_value(Segment const& self) {
        return self.value_;
    }

private:
    std::size_t value_;
};

} // end namespace d2

#endif // !D2_SEGMENT_HPP
