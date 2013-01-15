/**
 * This file defines the `Thread` class.
 */

#ifndef D2_THREAD_HPP
#define D2_THREAD_HPP

#include <d2/concepts.hpp>

#include <boost/concept/assert.hpp>
#include <boost/functional/hash.hpp>
#include <boost/operators.hpp>
#include <cstddef>
#include <iostream>


namespace d2 {

/**
 * Represents a thread in the analyzed program.
 * @note `Thread` only _represents_ a thread of the analyzed program, it
 *       isn't itself a thread class.
 */
class Thread : public boost::equality_comparable<Thread> {
    std::size_t id_;

public:
    /**
     * This constructor should only be used when serializing `Thread`s.
     * A default-constructed `Thread` is in an invalid state.
     */
    Thread()
        : id_(0)
    { }

    /**
     * Create a `Thread` representing the `UniquelyIdentifiable` object
     * `t`. While `t` is not _required_ to represent a thread, it makes no
     * sense to use `Thread` if it does not.
     */
    template <typename T>
    explicit Thread(T const& t) : id_(unique_id(t)) {
        BOOST_CONCEPT_ASSERT((UniquelyIdentifiable<T>));
    }

    /**
     * Construct a `Thread` refering to the same thread of the analyzed
     * program as `other`.
     */
    Thread(Thread const& other)
        : id_(other.id_)
    { }

    /**
     * Save a `Thread` to an output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, Thread const& t) {
        return os << t.id_, os;
    }

    /**
     * Load a `Thread` from an input stream.
     */
    friend std::istream& operator>>(std::istream& is, Thread& t) {
        return is >> t.id_, is;
    }

    /**
     * Compute a hash value uniquely representing the thread of the analyzed
     * program.
     * @note This is very useful to efficiently store `Thread`s in
     *       unordered containers.
     */
    friend std::size_t hash_value(Thread const& self) {
        using boost::hash_value;
        return hash_value(self.id_);
    }

    /**
     * Return whether two `Thread`s refer to the same thread of the analyzed
     * program.
     */
    friend bool operator==(Thread const& lhs, Thread const& rhs) {
        return lhs.id_ == rhs.id_;
    }
};

} // end namespace d2

#endif // !D2_THREAD_HPP
