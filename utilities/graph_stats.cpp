/**
 * This file implements a small utility to output statistics
 * about a lock graph.
 */

#include <d2/graph_construction.hpp>
#include <d2/logging.hpp>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphviz.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>


inline void usage() {
    std::cout << "Usage: graph_stats input_file\n";
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

    std::cout << "num_vertices : " << num_vertices(lg) << '\n'
              << "num_edges : " << num_edges(lg) << '\n';
}
