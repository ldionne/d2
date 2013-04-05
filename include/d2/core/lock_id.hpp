/**
 * This file defines the `LockId` class.
 */

#ifndef D2_CORE_LOCK_ID_HPP
#define D2_CORE_LOCK_ID_HPP

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
class LockId : public boost::equality_comparable<LockId,
                      boost::less_than_comparable<LockId> > {
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
     * Create a `LockId` representing a synchronization object identified by
     * `id`.
     *
     * @note Forcing the use of `std::size_t` moves the burden of obtaining
     *       and managing unique identifiers for all synchronization objects,
     *       which simplifies our job considerably.
     */
    explicit LockId(std::size_t)
        : id_(id)
    { }

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

    friend bool operator<(LockId const& self, LockId const& other) {
        return self.id_ < other.id_;
    }

    friend bool operator>(LockId const& self, LockId const& other) {
        return self.id_ > other.id_;
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

#endif // !D2_CORE_LOCK_ID_HPP
