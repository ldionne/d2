/**
 * This file implements a small utility to output statistics
 * about a lock graph.
 */

#include <d2/analysis.hpp>
#include <d2/graph_construction.hpp>
#include <d2/logging.hpp>

#include <boost/assert.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/optional.hpp>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>


template <typename LockGraph, typename SegmentationGraph>
struct Statistics {
    Statistics(LockGraph const& lg, SegmentationGraph const&)
        : number_of_lock_graph_vertices(num_vertices(lg)),
          number_of_lock_graph_edges(num_edges(lg))
    { }

    std::size_t number_of_lock_graph_vertices;
    std::size_t number_of_lock_graph_edges;
    boost::optional<std::size_t> number_of_distinct_cycles;

    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os, Statistics const& self) {
        os << "number of vertices in the lock graph: " << self.number_of_lock_graph_vertices << '\n'
           << "number of edges in the lock graph: " << self.number_of_lock_graph_edges << '\n'
           << "number of distinct cycles in the lock graph: ";

        if (self.number_of_distinct_cycles)
            os << *self.number_of_distinct_cycles;
        else
            os << "not computed";

        return os;
    }
};

template <typename Stats>
class StatisticGatherer {
    Stats& stats_;

public:
    explicit StatisticGatherer(Stats& stats) : stats_(stats) {
        BOOST_ASSERT_MSG(!stats_.number_of_distinct_cycles,
        "gathering statistics on a statistic object that was already filled");
        stats_.number_of_distinct_cycles = 0;
    }

    template <typename EdgePath, typename LockGraph>
    void operator()(EdgePath const&, LockGraph const&) const {
        ++*stats_.number_of_distinct_cycles;
    }
};

template <typename Stats>
StatisticGatherer<Stats> gather_stats(Stats& stats) {
    return StatisticGatherer<Stats>(stats);
}


int main(int argc, char const *argv[]) {
    std::ifstream ifs;
    if (argc == 2) {
        std::vector<std::string> args(argv, argv + argc);
        std::string input_file(args[1]);
        ifs.open(input_file.c_str());
        if (!ifs) {
            std::cout << "Unable to open input file \"" << input_file << "\"\n";
            return EXIT_FAILURE;
        }
    }
    std::istream& is(argc == 2 ? ifs : std::cin);

    d2::SegmentationGraph sg;
    d2::LockGraph lg;
    d2::build_graphs(d2::load_events(is), lg, sg);

    Statistics<d2::LockGraph, d2::SegmentationGraph> stats(lg, sg);
    d2::analyze(lg, sg, gather_stats(stats));

    std::cout << stats << std::endl;
}
