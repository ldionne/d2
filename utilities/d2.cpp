/**
 * This file implements a command line utility to interact
 * with the d2 library.
 */

#include <d2/analysis.hpp>
#include <d2/detail/config.hpp>
#include <d2/graph_construction.hpp>

#include <boost/assert.hpp>
#include <boost/assign.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/properties.hpp>
#include <boost/optional.hpp>
#include <boost/phoenix.hpp>
#include <boost/program_options.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>


static std::string const VERSION = "0.1a";

namespace po = boost::program_options;
namespace phx = boost::phoenix;
using phx::arg_names::_1;

namespace {

// Dot rendering
template <typename LockGraph>
class LockGraphWriter {
    typedef typename boost::graph_traits<LockGraph>::edge_descriptor
                                                            EdgeDescriptor;
    typedef typename boost::graph_traits<LockGraph>::vertex_descriptor
                                                            VertexDescriptor;
    LockGraph const& graph_;

public:
    explicit LockGraphWriter(LockGraph const& lg) : graph_(lg) { }

    template <typename Ostream>
    void operator()(Ostream& os, EdgeDescriptor edge) const {
        os << "[label=\""
           << "from " << graph_[edge].l1_info.call_stack[0].function

           << " to " << graph_[edge].l2_info.call_stack[0].function
           << "\"]";
    }

    template <typename Ostream>
    void operator()(Ostream&, VertexDescriptor) const {

    }
};

template <typename LockGraph>
LockGraphWriter<LockGraph> render_dot(LockGraph const& lg) {
    return LockGraphWriter<LockGraph>(lg);
}

// Statistic gathering
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

// Deadlock analysis
template <typename Ostream>
class CyclePrinter {
    Ostream& os_;

    void format_call_stack(d2::detail::LockDebugInfo const& info,
                           std::string const& indent = "") const {
        BOOST_FOREACH(d2::detail::StackFrame const& frame, info.call_stack) {
            os_ << indent << frame << '\n';
        }
    }

public:
    explicit CyclePrinter(Ostream& os) : os_(os) { }

    template <typename EdgePath, typename LockGraph>
    void operator()(EdgePath const& cycle, LockGraph const& graph) const {
        typedef typename boost::graph_traits<LockGraph>::edge_descriptor
                                                    LockGraphEdgeDescriptor;
        typedef typename boost::edge_property<LockGraph>::type LockGraphEdge;
        os_ << "----------------------------------------------------";

        BOOST_FOREACH(LockGraphEdgeDescriptor const& edge_desc, cycle) {
            LockGraphEdge const& edge = graph[edge_desc];
            d2::SyncObject const& l1 = graph[source(edge_desc, graph)];
            d2::SyncObject const& l2 = graph[target(edge_desc, graph)];

            os_ << '\n';

            os_ << "Thread " << edge.t << " acquired lock " << l2 << " in\n";
            format_call_stack(edge.l2_info, "    ");

            os_ << "while holding lock " << l1 << " taken in\n";
            format_call_stack(edge.l1_info, "    ");
        }

        os_ << "----------------------------------------------------\n\n";
    }
};

template <typename Ostream>
CyclePrinter<Ostream> print_cycle(Ostream& os) {
    return CyclePrinter<Ostream>(os);
}

} // end anonymous namespace


int main(int argc, char const* argv[]) {
    // Parse command line options.
    po::options_description global("Global options");
    global.add_options()
    (
        "help,h", "produce help message and exit"
    )(
        "version,v", "print version and exit"
    )(
        "output-file,o", po::value<std::string>(), "file to write the output to"
    )(
        "analyze", "perform the analysis for deadlocks"
    )(
        "dot", "produce a dot representation of the lock graph used during the analysis"
    )(
        "stats", "produce statistics about the usage of locks and threads"
    )
    ;

    po::options_description hidden;
    hidden.add_options()
        ("input-file", po::value<std::string>(), "input file to process")
    ;
    po::positional_options_description positionals;
    positionals.add("input-file", 0);

    po::options_description analysis("Analysis options");

    po::options_description dot("Dot rendering options");

    po::options_description statistics("Statistics options");

    po::options_description allowed;
    allowed.add(global).add(analysis).add(dot).add(statistics);

    po::options_description all;
    all.add(allowed).add(hidden);

    po::variables_map args;
    po::command_line_parser parser(argc, argv);
    po::store(parser.options(all).positional(positionals).run(), args);

    // Some options make us do something and exit right away. These cases
    // are handled here.
    {
        typedef boost::function<void(po::variable_value)> DieAction;
        std::map<std::string, DieAction> die_actions =
        boost::assign::map_list_of<std::string, DieAction>
            ("help", phx::ref(std::cout) << allowed << '\n')
            ("version", phx::ref(std::cout) << VERSION << '\n')
        ;
        typedef std::pair<std::string, DieAction> DieActionPair;
        BOOST_FOREACH(DieActionPair const& action, die_actions) {
            if (args.count(action.first)) {
                action.second(args[action.first]);
                return EXIT_FAILURE;
            }
        }
    }

    // Open the input/output streams to the specified files or their
    // defaults if unspecified.
    std::ifstream input_ifs;
    if (args.count("input-file")) {
        std::string input_file = args["input-file"].as<std::string>();
        input_ifs.open(input_file.c_str());
        if (!input_ifs) {
            std::cerr << "Unable to open input file \"" << input_file << '"';
            return EXIT_FAILURE;
        }
    }
    std::istream& input = args.count("input-file") ? input_ifs : std::cin;

    std::ofstream output_ofs;
    if (args.count("output-file")) {
        std::string output_file = args["output-file"].as<std::string>();
        output_ofs.open(output_file.c_str());
        if (!output_ofs) {
            std::cerr << "Unable to open output file \"" << output_file <<'"';
            return EXIT_FAILURE;
        }
    }
    std::ostream& output = args.count("output-file") ? output_ofs : std::cout;

    // Build the segmentation and the lock graph. They are required no matter
    // the options we received on the command line.
    d2::SegmentationGraph sg;
    d2::LockGraph lg;
    d2::build_graphs(d2::load_events(input), lg, sg);

    // Main switch dispatching to the right action to perform.
    if (args.count("analyze") || (!args.count("dot") && !args.count("stats"))) {
        d2::analyze(lg, sg, print_cycle(std::cout));
    }
    else if (args.count("dot")) {
        boost::write_graphviz(output, lg, render_dot(lg), render_dot(lg));
    }
    else if (args.count("stats")) {
        Statistics<d2::LockGraph, d2::SegmentationGraph> stats(lg, sg);
        d2::analyze(lg, sg, gather_stats(stats));
        output << stats << std::endl;
    }
    return EXIT_SUCCESS;
}
