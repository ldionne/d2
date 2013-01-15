/**
 * This file implements a command line utility to interact
 * with the d2 library.
 */

#include <d2/analysis.hpp>
#include <d2/event_repository.hpp>
#include <d2/graph_construction.hpp>

#include <boost/assert.hpp>
#include <boost/assign.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/function.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/properties.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/phoenix.hpp>
#include <boost/program_options.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <boost/scoped_ptr.hpp>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>


static std::string const VERSION = "0.1a";

namespace fs = boost::filesystem;
namespace phx = boost::phoenix;
namespace po = boost::program_options;
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

    // Silence MSVC warning C4512: assignment operator could not be generated
    LockGraphWriter& operator=(LockGraphWriter const&) /*= delete*/;

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

    // Silence MSVC warning C4512: assignment operator could not be generated
    StatisticGatherer& operator=(StatisticGatherer const&) /*= delete*/;

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

    // Silence MSVC warning C4512: assignment operator could not be generated
    CyclePrinter& operator=(CyclePrinter const&) /*= delete*/;

    void format_call_stack(d2::detail::LockDebugInfo const& info,
                           std::string const& indent = "") const {
        BOOST_FOREACH(d2::detail::StackFrame const& frame, info.call_stack) {
            os_ << indent
            << frame.ip << "    " << frame.function << " in " << frame.module
            << '\n';
        }
    }

public:
    explicit CyclePrinter(Ostream& os) : os_(os) { }

    template <typename EdgePath, typename LockGraph>
    void operator()(EdgePath const& cycle, LockGraph const& graph) const {
        typedef typename boost::graph_traits<LockGraph>::edge_descriptor
                                                    LockGraphEdgeDescriptor;
        typedef typename boost::edge_property_type<LockGraph>::type
                                                                LockGraphEdge;
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

template <typename ErrorTag, typename Exception>
std::string get_error_info(Exception const& e,
                           std::string const& default_ = "\"unavailable\"") {
    typename ErrorTag::value_type const* info =
                                        boost::get_error_info<ErrorTag>(e);
    return info ? boost::lexical_cast<std::string>(*info) : default_;
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
        ("repo-path", po::value<std::string>(), "path of the repository to examine")
        ("debug", "enable special debugging output")
    ;
    po::positional_options_description positionals;
    positionals.add("repo-path", 1);

    po::options_description analysis("Analysis options");

    po::options_description dot("Dot rendering options");

    po::options_description statistics("Statistics options");

    po::options_description allowed;
    allowed.add(global).add(analysis).add(dot).add(statistics);

    po::options_description all;
    all.add(allowed).add(hidden);

    po::variables_map args;
    po::command_line_parser parser(argc, argv);

    try { // begin top-level try/catch all

    try {
        po::store(parser.options(all).positional(positionals).run(), args);
        po::notify(args);
    } catch (po::error const& e) {
        std::cerr << e.what() << std::endl
                  << allowed << std::endl;
        return EXIT_FAILURE;
    }

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

    // Open the repository on which we must operate.
    if (!args.count("repo-path")) {
        std::cerr << "missing the path of a repository on which to operate "
                     "from the command line" << std::endl
                  << allowed << std::endl;
        return EXIT_FAILURE;
    }
    fs::path repo_path = args["repo-path"].as<std::string>();
    if (!fs::exists(repo_path)) {
        std::cerr << "the repository path " << repo_path << " does not exist\n";
        return EXIT_FAILURE;
    }

    boost::scoped_ptr<d2::EventRepository<> > repository;
    try {
        repository.reset(new d2::EventRepository<>(repo_path));
    } catch (d2::RepositoryException const& e) {
        std::cerr << "unable to open the repository at " << repo_path << '\n';
        if (args.count("debug"))
            std::cerr << boost::diagnostic_information(e) << '\n';
        return EXIT_FAILURE;
    }

    // Open the output stream to whatever passed on the command line or to
    // stdout if unspecified.
    std::ofstream output_ofs;
    if (args.count("output-file")) {
        std::string output_file = args["output-file"].as<std::string>();
        output_ofs.open(output_file.c_str());
        if (!output_ofs) {
            std::cerr << "unable to open output file \"" << output_file << '"';
            return EXIT_FAILURE;
        }
    }
    std::ostream& output = args.count("output-file") ? output_ofs : std::cout;

    // Build the segmentation and the lock graph. They are required no matter
    // the options we received on the command line.
    d2::SegmentationGraph sg;
    d2::LockGraph lg;
    try {
        d2::build_graphs(*repository, lg, sg);
    } catch (d2::EventTypeException const& e) {
        std::string actual_type = get_error_info<d2::ActualType>(e);
        std::string expected_type = get_error_info<d2::ExpectedType>(e);

        std::cerr <<
        "error while building the graphs:\n"
        "    encountered an event of type " << actual_type << '\n' <<
        "    while expecting an event of type " << expected_type << '\n';
        return EXIT_FAILURE;

    } catch (d2::UnexpectedReleaseException const& e) {
        std::string lock = get_error_info<d2::ReleasedLock>(e);
        std::string thread = get_error_info<d2::ReleasingThread>(e);

        std::cerr <<
        "error while building the graphs:\n" <<
        "    lock " << lock <<
                    " was unexpectedly released by thread " << thread << '\n';
         if (args.count("debug"))
            std::cerr << boost::diagnostic_information(e) << '\n';
        return EXIT_FAILURE;
    }

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

    // end top-level try/catch all
    } catch (std::exception const& e) {
        std::cerr << "an unknown problem has happened, please contact the devs\n";
        if (args.count("debug"))
            std::cerr << boost::diagnostic_information(e) << '\n';
        return EXIT_FAILURE;

    }
}
