/**
 * This file defines the `Segment` class.
 */

#ifndef D2_CORE_SEGMENT_HPP
#define D2_CORE_SEGMENT_HPP

#include <boost/operators.hpp>
#include <boost/serialization/access.hpp>
#include <cstddef>
#include <iostream>


namespace d2 {

/**
 * Represents a segment inside which operations happen in a serialized
 * manner inside a program.
 */
struct Segment : boost::totally_ordered<Segment,
                 boost::additive<Segment, std::size_t,
                 boost::unit_steppable<Segment> > > {

    Segment()
        : value_(0)
    { }

    friend bool operator==(Segment const& a, Segment const& b) {
        return a.value_ == b.value_;
    }

    friend bool operator<(Segment const& a, Segment const& b) {
        return a.value_ < b.value_;
    }

    friend bool operator>(Segment const& a, Segment const& b) {
        return a.value_ > b.value_;
    }

    friend Segment& operator+=(Segment& a, std::size_t amount) {
        a.value_ += amount;
        return a;
    }

    friend Segment& operator-=(Segment& a, std::size_t amount) {
        a.value_ -= amount;
        return a;
    }

    Segment& operator++() {
        ++value_;
        return *this;
    }

    Segment& operator--() {
        --value_;
        return *this;
    }

    friend std::ostream& operator<<(std::ostream& os, Segment const& self) {
        return os << self.value_, os;
    }

    friend std::istream& operator>>(std::istream& is, Segment& self) {
        return is >> self.value_, is;
    }

    friend std::size_t hash_value(Segment const& self) {
        return self.value_;
    }

private:
    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, unsigned int const) {
        ar & value_;
    }

    std::size_t value_;
};

} // end namespace d2

#endif // !D2_CORE_SEGMENT_HPP
