
#include <d2/analysis.hpp>
#include <d2/graph_construction.hpp>
#include <d2/logging.hpp>

#include <boost/foreach.hpp>
#include <boost/graph/properties.hpp>
#include <boost/range/begin.hpp>
#include <boost/range/end.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>


template <typename Ostream>
class print_cycle_type {
    Ostream& os_;

public:
    explicit print_cycle_type(Ostream& os) : os_(os) { }

    template <typename EdgePath, typename LockGraph>
    void operator()(EdgePath const& cycle, LockGraph const& graph) const {
        typedef typename boost::graph_traits<LockGraph>::edge_descriptor
                                                    LockGraphEdgeDescriptor;
        typedef typename boost::edge_property<LockGraph>::type LockGraphEdge;

        BOOST_FOREACH(LockGraphEdgeDescriptor const& edge_desc, cycle) {
            LockGraphEdge const& edge = graph[edge_desc];
            os_ << edge.l1_info.file << ':' << edge.l1_info.line << '\n'
                << edge.l2_info.file << ':' << edge.l2_info.line << '\n';
        }
        os_ << '\n';
    }
};

template <typename Ostream>
print_cycle_type<Ostream> print_cycle(Ostream& os) {
    return print_cycle_type<Ostream>(os);
}

inline void usage() {
    std::cout << "Usage: prog input_file\n";
}

int main(int argc, char const *argv[]) {
    if (argc < 2) return usage(), EXIT_FAILURE;
    std::vector<std::string> args(argv, argv + argc);
    std::string input_file(args[1]);
    std::ifstream ifs(input_file.c_str());
    if (!ifs) {
        std::cout << "Unable to open input file \"" << input_file << "\"\n";
        return EXIT_FAILURE;
    }

    d2::segmentation_graph sg;
    d2::lock_graph lg;
    d2::build_graphs(d2::load_events(ifs), lg, sg);
    d2::analyze(lg, sg, print_cycle(std::cout));
}
