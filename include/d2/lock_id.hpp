/**
 * This file defines the `LockId` class.
 */

#ifndef D2_LOCK_ID_HPP
#define D2_LOCK_ID_HPP

#include <d2/concepts.hpp>

#include <boost/concept/assert.hpp>
#include <boost/functional/hash.hpp>
#include <boost/operators.hpp>
#include <boost/serialization/access.hpp>
#include <cstddef>
#include <iostream>


namespace d2 {

/**
 * Unique id representing a synchronization object in the analyzed program.
 */
class LockId : public boost::equality_comparable<LockId> {
    std::size_t id_;

    friend class boost::serialization::access;
    template <typename Archive>
    void serialize(Archive& ar, unsigned int const) {
        ar & id_;
    }

public:
    /**
     * This constructor should only be used when serializing `LockId`s.
     * A default-constructed `LockId` is in an invalid state.
     */
    LockId()
        : id_(0)
    { }

    /**
     * Create a `LockId` representing the `UniquelyIdentifiable` object `t`.
     * While `t` is not _required_ to represent a synchronization object, it
     * makes no sense to use `LockId` if it does not.
     */
    template <typename T>
    explicit LockId(T const& t) : id_(unique_id(t)) {
        BOOST_CONCEPT_ASSERT((UniquelyIdentifiable<T>));
    }

    /**
     * Construct a `LockId` refering to the same synchronization object
     * in the analyzed program as `other`.
     */
    LockId(LockId const& other)
        : id_(other.id_)
    { }

    /**
     * Save a `LockId` to an output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, LockId const& self) {
        return os << self.id_, os;
    }

    /**
     * Load a `LockId` from an input stream.
     */
    friend std::istream& operator>>(std::istream& is, LockId& self) {
        return is >> self.id_, is;
    }

    /**
     * Compute a hash value uniquely representing the synchronization object
     * in the analyzed program.
     */
    friend std::size_t hash_value(LockId const& self) {
        using boost::hash_value;
        return hash_value(self.id_);
    }

    /**
     * Return whether two `LockId`s refer to the same synchronization
     * objects in the analyzed program.
     */
    friend bool operator==(LockId const& lhs, LockId const& rhs) {
        return lhs.id_ == rhs.id_;
    }
};

} // end namespace d2

#endif // !D2_LOCK_ID_HPP
