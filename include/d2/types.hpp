/**
 * This file defines several types used in the library.
 */

#ifndef D2_TYPES_HPP
#define D2_TYPES_HPP

#include <d2/detail/support.hpp>

#include <boost/concept_check.hpp>
#include <boost/functional/hash.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/named_graph.hpp>
#include <boost/mpl/and.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/operators.hpp>
#include <boost/static_assert.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/unordered_set.hpp>
#include <boost/utility/enable_if.hpp>
#include <cstddef>


namespace d2 {

/**
 * Partial specialization of `unique_id` for unsigned integral types. For
 * these types, `unique_id` is the identity function.
 */
template <typename T>
typename boost::enable_if<
            boost::mpl::and_<boost::is_integral<T>, boost::is_unsigned<T> >,
std::size_t>::type unique_id(T const& t) {
    return static_cast<std::size_t>(t);
}

/**
 * Concept specification of the `UniquelyIdentifiable` concept.
 *
 * A type is `UniquelyIdentifiable` iff it is possible to obtain an unsigned
 * integral identifier that is unique for any two distinct objects. This is
 * much like being able to hash an object, but the hash has to be perfect.
 */
template <typename T>
struct UniquelyIdentifiable {
    BOOST_CONCEPT_USAGE(UniquelyIdentifiable) {
        using ::d2::unique_id;
        std::size_t id = unique_id(val);
        (void)id;
    }

    T const& val;
};

/**
 * Archetype for the `UniquelyIdentifiable` concept.
 */
class uniquely_identifiable_archetype {
    typedef uniquely_identifiable_archetype self_type;
    uniquely_identifiable_archetype() /*= delete*/;
    uniquely_identifiable_archetype(self_type const&) /*= delete*/;
    self_type& operator=(self_type const&) /*= delete*/;
    ~uniquely_identifiable_archetype() /*= delete*/;
public:
    friend std::size_t unique_id(self_type const&);
};

/**
 * Represents a synchronization object in a program. Examples of
 * synchronization objects are a mutex and a semaphore.
 *
 * It is important to understand how `sync_object` only _represents_ a
 * synchronization object without being one. Representing synchronization
 * objects is useful when performing a post-mortem analysis of the execution
 * of a program.
 */
class sync_object : public boost::equality_comparable<sync_object> {
    std::size_t id_;

public:
    /**
     * This constructor should only be used when serializing `sync_object`s.
     * A default-constructed `sync_object` is in an invalid state.
     */
    sync_object() { }

    /**
     * Save a `sync_object` to an output stream.
     */
    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os, sync_object const& s) {
        return os << s.id_, os;
    }

    /**
     * Load a `sync_object` from an input stream.
     */
    template <typename Istream>
    friend Istream& operator>>(Istream& is, sync_object& s) {
        return is >> s.id_, is;
    }

    /**
     * Create a `sync_object` representing the `UniquelyIdentifiable` object
     * `t`. While `t` is not _required_ to represent a synchronization object,
     * it makes no sense to use `sync_object` if it does not.
     */
    template <typename T>
    explicit sync_object(T const& t) : id_(unique_id(t)) {
        BOOST_CONCEPT_ASSERT((UniquelyIdentifiable<T>));
    }

    /**
     * Construct a `sync_object` refering to the same synchronization object
     * in the analyzed program as `other`.
     */
    inline sync_object(sync_object const& other) : id_(other.id_) { }

    /**
     * Compute a hash value uniquely representing the synchronization object
     * in the analyzed program.
     * @note This is very useful to efficiently store `sync_object`s in
     *       unordered containers.
     */
    friend std::size_t hash_value(sync_object const& self) {
        using boost::hash_value;
        return hash_value(self.id_);
    }

    /**
     * Return whether two `sync_object`s refer to the same synchronization
     * objects in the analyzed program.
     */
    friend bool operator==(sync_object const& lhs, sync_object const& rhs) {
        return lhs.id_ == rhs.id_;
    }
};

/**
 * Represents a thread in a program.
 *
 * Much like `sync_object`s, this is useful to represent a thread during a
 * post-mortem analysis of a program.
 */
class thread : public boost::equality_comparable<thread> {
    std::size_t id_;

public:
    /**
     * This constructor should only be used when serializing `thread`s.
     * A default-constructed `thread` is in an invalid state.
     */
    thread() { }

    /**
     * Save a `thread` to an output stream.
     */
    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os, thread const& t) {
        return os << t.id_, os;
    }

    /**
     * Load a `thread` from an input stream.
     */
    template <typename Istream>
    friend Istream& operator>>(Istream& is, thread& t) {
        return is >> t.id_, is;
    }

    /**
     * Create a `thread` representing the `UniquelyIdentifiable` object
     * `t`. While `t` is not _required_ to represent a thread, it makes no
     * sense to use `thread` if it does not.
     */
    template <typename T>
    explicit thread(T const& t) : id_(unique_id(t)) {
        BOOST_CONCEPT_ASSERT((UniquelyIdentifiable<T>));
    }

    /**
     * Construct a `thread` refering to the same thread of the analyzed
     * program as `other`.
     */
    inline thread(thread const& other) : id_(other.id_) { }

    /**
     * Compute a hash value uniquely representing the thread of the analyzed
     * program.
     * @note This is very useful to efficiently store `thread`s in
     *       unordered containers.
     */
    friend std::size_t hash_value(thread const& self) {
        using boost::hash_value;
        return hash_value(self.id_);
    }

    /**
     * Return whether two `thread`s refer to the same thread of the analyzed
     * program.
     */
    friend bool operator==(thread const& lhs, thread const& rhs) {
        return lhs.id_ == rhs.id_;
    }
};


/**
 * Directed acyclic graph representing the order of starts and joins between
 * the threads of a program.
 */
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS>
                                                        segmentation_graph;
typedef boost::graph_traits<segmentation_graph>::vertex_descriptor segment;

/**
 * Label present on the edges of a lock graph.
 */
struct lock_graph_label {
    detail::lock_debug_info l1_info;
    segment s1;
    thread t;
    boost::unordered_set<sync_object> g;
    segment s2;
    detail::lock_debug_info l2_info;
};

/**
 * Directed graph representing the contexts in which synchronization objects
 * were acquired by threads.
 */
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
                                    sync_object, lock_graph_label> lock_graph;

/**
 * Concept specification of a lock graph.
 */
template <typename G>
struct LockGraphConcept : boost::GraphConcept<G> {
    BOOST_STATIC_ASSERT((::boost::is_same<
                            typename boost::edge_property_type<G>::type,
                            lock_graph_label
                        >::value));

    BOOST_STATIC_ASSERT((::boost::is_same<
                            typename boost::vertex_property_type<G>::type,
                            sync_object
                        >::value));
};

} // end namespace d2

namespace boost {
namespace graph {
    // This is to be able to refer to a vertex in the lock graph using the
    // sync_object associated to it.
    template <> struct internal_vertex_name<d2::sync_object> {
        typedef multi_index::identity<d2::sync_object> type;
    };

    // This is to satisfy the EdgeIndexGraph concept, which is
    // BOOST_CONCEPT_ASSERTed in tiernan_all_cycles even though
    // it is not required.
    void renumber_vertex_indices(d2::lock_graph const&);
} // end namespace graph
} // end namespace boost

#endif // !D2_TYPES_HPP
