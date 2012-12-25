
#include <d2/graph_construction.hpp>
#include <d2/logging.hpp>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/phoenix.hpp>
#include <boost/program_options.hpp>
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

// cello::apply(phx::bind(&std::ifstream::open, input_file, _1)) >>= &cello::on::operator=

namespace po = boost::program_options;
namespace phx = boost::phoenix;
using phx::arg_names::_1;

int main(int argc, char const *argv[]) {
    std::string input_file, output_file;

    po::options_description general("Available options");
    general.add_options()
        (
            "input-file,f", po::value<std::string>(&input_file),
            "input file containing the logs from which to "
            "create a dot representation"
        )(
            "output-file,o", po::value<std::string>(&output_file),
            "output file for the dot representation"
        )
    ;
    po::positional_options_description positionals;
    positionals.add("input-file", 1);

    po::variables_map args;
    po::store(po::command_line_parser(argc, argv).
        options(general).positional(positionals).run(), args);
    po::notify(args);

    std::ifstream input_ifs;
    if (!input_file.empty()) {
        input_ifs.open(input_file.c_str());
        if (!input_ifs) {
            std::cerr << "Unable to open input file \"" << input_file << '"';
            return EXIT_FAILURE;
        }
    }
    std::istream& input = input_file.empty() ? std::cin : input_ifs;

    std::ofstream output_ofs;
    if (!output_file.empty()) {
        output_ofs.open(output_file.c_str());
        if (!output_ofs) {
            std::cerr << "Unable to open output file \"" << output_file <<'"';
            return EXIT_FAILURE;
        }
    }
    std::ostream& output = output_file.empty() ? std::cout : output_ofs;

    d2::SegmentationGraph sg;
    d2::LockGraph lg;
    d2::build_graphs(d2::load_events(input), lg, sg);

    LockGraphWriter<d2::LockGraph> writer(lg);
    boost::write_graphviz(output, lg, writer, writer);
}
