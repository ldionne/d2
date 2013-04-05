/**
 * This file defines the `ThreadId` class.
 */

#ifndef D2_CORE_THREAD_ID_HPP
#define D2_CORE_THREAD_ID_HPP

#include <boost/concept/assert.hpp>
#include <boost/functional/hash.hpp>
#include <boost/operators.hpp>
#include <boost/serialization/access.hpp>
#include <cstddef>
#include <iostream>


namespace d2 {

/**
 * Unique id identifying a thread in the analyzed program.
 */
class ThreadId : public boost::equality_comparable<ThreadId,
                        boost::less_than_comparable<ThreadId> > {
    std::size_t id_;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, unsigned int const) {
        ar & id_;
    }

public:
    /**
     * This constructor should only be used when serializing `ThreadId`s.
     * A default-constructed `ThreadId` is in an invalid state.
     */
    ThreadId()
        : id_(0)
    { }

    /**
     * Create a `ThreadId` representing a thread identified by `id`.
     *
     * @note Forcing the use of `std::size_t` moves the burden of making
     *       sure it is possible to obtain thread ids as integral types
     *       portably to the client of `d2::core`, which is a good thing.
     */
    explicit ThreadId(std::size_t id)
        : id_(id)
    { }

    /**
     * Construct a `ThreadId` refering to the same thread of the analyzed
     * program as `other`.
     */
    ThreadId(ThreadId const& other)
        : id_(other.id_)
    { }

    /**
     * Save a `ThreadId` to an output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, ThreadId const& self) {
        return os << self.id_, os;
    }

    /**
     * Load a `ThreadId` from an input stream.
     */
    friend std::istream& operator>>(std::istream& is, ThreadId& self) {
        return is >> self.id_, is;
    }

    /**
     * Compute a hash value uniquely representing the thread of the analyzed
     * program.
     */
    friend std::size_t hash_value(ThreadId const& self) {
        using boost::hash_value;
        return hash_value(self.id_);
    }

    friend bool operator<(ThreadId const& self, ThreadId const& other) {
        return self.id_ < other.id_;
    }

    friend bool operator>(ThreadId const& self, ThreadId const& other) {
        return self.id_ > other.id_;
    }

    /**
     * Return whether two `ThreadId`s refer to the same thread of the analyzed
     * program.
     */
    friend bool operator==(ThreadId const& lhs, ThreadId const& rhs) {
        return lhs.id_ == rhs.id_;
    }
};

} // end namespace d2

#endif // !D2_CORE_THREAD_ID_HPP
