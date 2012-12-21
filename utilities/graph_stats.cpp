/**
 * This file implements a small utility to output statistics
 * about a lock graph.
 */

#include <d2/graph_construction.hpp>
#include <d2/logging.hpp>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphviz.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>


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

    std::cout << "num_vertices : " << num_vertices(lg) << '\n'
              << "num_edges : " << num_edges(lg) << '\n';
}
