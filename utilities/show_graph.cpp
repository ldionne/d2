
#include <d2/graph_construction.hpp>
#include <d2/logging.hpp>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphviz.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>


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
           << "from " << graph_[edge].l1_info.file << ':'
                      << graph_[edge].l1_info.line << ' '

           << "to " << graph_[edge].l2_info.file << ':'
                    << graph_[edge].l2_info.line
           << "\"]";
    }

    template <typename Ostream>
    void operator()(Ostream&, VertexDescriptor) const {

    }
};

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

    d2::segmentation_graph sg;
    d2::lock_graph lg;
    d2::build_graphs(d2::load_events(is), lg, sg);

    LockGraphWriter<d2::lock_graph> writer(lg);
    boost::write_graphviz(std::cout, lg, writer, writer);
}
