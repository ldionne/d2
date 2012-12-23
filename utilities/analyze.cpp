
#include <d2/analysis.hpp>
#include <d2/graph_construction.hpp>
#include <d2/logging.hpp>

#include <boost/foreach.hpp>
#include <boost/graph/properties.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>


template <typename Ostream>
class CyclePrinter {
    Ostream& os_;

    void format_call_stack(d2::detail::lock_debug_info const& info,
                           std::string const& indent = "") const {
        BOOST_FOREACH(std::string const& frame, info.call_stack) {
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
    d2::analyze(lg, sg, print_cycle(std::cout));
}
