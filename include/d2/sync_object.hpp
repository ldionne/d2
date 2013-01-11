/**
 * This file defines the `SyncObject` class.
 */

#ifndef D2_SYNC_OBJECT_HPP
#define D2_SYNC_OBJECT_HPP

#include <d2/concepts.hpp>

#include <boost/concept/assert.hpp>
#include <boost/functional/hash.hpp>
#include <boost/operators.hpp>
#include <cstddef>
#include <iosfwd>


namespace d2 {

/**
 * Represents a synchronization object in the analyzed program. Examples of
 * synchronization objects are a mutex, a semaphore and a spinlock.
 * @note `SyncObject` only _represents_ a synchronization object, it can't be
 *       used itself as a synchronization mechanism.
 */
class SyncObject : public boost::equality_comparable<SyncObject> {
    std::size_t id_;

public:
    /**
     * This constructor should only be used when serializing `SyncObject`s.
     * A default-constructed `SyncObject` is in an invalid state.
     */
    inline SyncObject() { }

    /**
     * Create a `SyncObject` representing the `UniquelyIdentifiable` object
     * `t`. While `t` is not _required_ to represent a synchronization object,
     * it makes no sense to use `SyncObject` if it does not.
     */
    template <typename T>
    explicit SyncObject(T const& t) : id_(unique_id(t)) {
        BOOST_CONCEPT_ASSERT((UniquelyIdentifiable<T>));
    }

    /**
     * Construct a `SyncObject` refering to the same synchronization object
     * in the analyzed program as `other`.
     */
    inline SyncObject(SyncObject const& other) : id_(other.id_) { }

    /**
     * Save a `SyncObject` to an output stream.
     */
    friend std::ostream& operator<<(std::ostream& os, SyncObject const& s) {
        return os << s.id_, os;
    }

    /**
     * Load a `SyncObject` from an input stream.
     */
    friend std::istream& operator>>(std::istream& is, SyncObject& s) {
        return is >> s.id_, is;
    }

    /**
     * Compute a hash value uniquely representing the synchronization object
     * in the analyzed program.
     * @note This is very useful to efficiently store `SyncObject`s in
     *       unordered containers.
     */
    friend std::size_t hash_value(SyncObject const& self) {
        using boost::hash_value;
        return hash_value(self.id_);
    }

    /**
     * Return whether two `SyncObject`s refer to the same synchronization
     * objects in the analyzed program.
     */
    friend bool operator==(SyncObject const& lhs, SyncObject const& rhs) {
        return lhs.id_ == rhs.id_;
    }
};

} // end namespace d2

#endif // !D2_SYNC_OBJECT_HPP
