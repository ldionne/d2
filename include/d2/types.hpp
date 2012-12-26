/**
 * This file defines several types used in the library.
 */

#ifndef D2_TYPES_HPP
#define D2_TYPES_HPP

#include <d2/detail/lock_debug_info.hpp>

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
#include <boost/type_traits/is_integral.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/is_unsigned.hpp>
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
 * It is important to understand how `SyncObject` only _represents_ a
 * synchronization object without being one. Representing synchronization
 * objects is useful when performing a post-mortem analysis of the execution
 * of a program.
 */
class SyncObject : public boost::equality_comparable<SyncObject> {
    std::size_t id_;

public:
    /**
     * This constructor should only be used when serializing `SyncObject`s.
     * A default-constructed `SyncObject` is in an invalid state.
     */
    SyncObject() { }

    /**
     * Save a `SyncObject` to an output stream.
     */
    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os, SyncObject const& s) {
        return os << s.id_, os;
    }

    /**
     * Load a `SyncObject` from an input stream.
     */
    template <typename Istream>
    friend Istream& operator>>(Istream& is, SyncObject& s) {
        return is >> s.id_, is;
    }

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

/**
 * Represents a thread in a program.
 *
 * Much like `SyncObject`s, this is useful to represent a thread during a
 * post-mortem analysis of a program.
 */
class Thread : public boost::equality_comparable<Thread> {
    std::size_t id_;

public:
    /**
     * This constructor should only be used when serializing `Thread`s.
     * A default-constructed `Thread` is in an invalid state.
     */
    Thread() { }

    /**
     * Save a `Thread` to an output stream.
     */
    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os, Thread const& t) {
        return os << t.id_, os;
    }

    /**
     * Load a `Thread` from an input stream.
     */
    template <typename Istream>
    friend Istream& operator>>(Istream& is, Thread& t) {
        return is >> t.id_, is;
    }

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
    inline Thread(Thread const& other) : id_(other.id_) { }

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


/**
 * Directed acyclic graph representing the order of starts and joins between
 * the threads of a program.
 */
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS>
                                                        SegmentationGraph;
typedef boost::graph_traits<SegmentationGraph>::vertex_descriptor Segment;

/**
 * Label present on the edges of a lock graph.
 */
struct LockGraphLabel : boost::equality_comparable<LockGraphLabel> {
    friend bool operator==(LockGraphLabel const& a, LockGraphLabel const& b) {
        // Note: We test the easiest first, i.e. the threads and segments,
        //       which are susceptible of being similar to integers.
        return a.s1 == b.s1 &&
               a.t == b.t &&
               a.s2 == b.s2 &&
               a.l1_info == b.l1_info &&
               a.l2_info == b.l2_info &&
               a.g == b.g;
    }

    inline LockGraphLabel() { }

    inline LockGraphLabel(detail::LockDebugInfo const& l1_info,
                          Segment s1,
                          Thread t,
                          boost::unordered_set<SyncObject> const& g,
                          Segment s2,
                          detail::LockDebugInfo const& l2_info)
        : l1_info(l1_info), s1(s1), t(t), g(g), s2(s2), l2_info(l2_info)
    { }

    detail::LockDebugInfo l1_info;
    Segment s1;
    Thread t;
    boost::unordered_set<SyncObject> g;
    Segment s2;
    detail::LockDebugInfo l2_info;
};

/**
 * Directed graph representing the contexts in which synchronization objects
 * were acquired by threads.
 */
typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
                                        SyncObject, LockGraphLabel> LockGraph;

/**
 * Concept specification of a lock graph.
 */
template <typename G>
struct LockGraphConcept : boost::GraphConcept<G> {
    BOOST_STATIC_ASSERT((::boost::is_same<
                            typename boost::edge_property_type<G>::type,
                            LockGraphLabel
                        >::value));

    BOOST_STATIC_ASSERT((::boost::is_same<
                            typename boost::vertex_property_type<G>::type,
                            SyncObject
                        >::value));
};

} // end namespace d2

namespace boost {
namespace graph {
    // This is to be able to refer to a vertex in the lock graph using the
    // SyncObject associated to it.
    template <> struct internal_vertex_name<d2::SyncObject> {
        typedef multi_index::identity<d2::SyncObject> type;
    };

    // This is to satisfy the EdgeIndexGraph concept, which is
    // BOOST_CONCEPT_ASSERTed in tiernan_all_cycles even though
    // it is not required.
    void renumber_vertex_indices(d2::LockGraph const&);
} // end namespace graph
} // end namespace boost

#endif // !D2_TYPES_HPP
