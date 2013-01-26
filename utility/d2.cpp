/**
 * This file implements a command line utility to interact
 * with the d2 library.
 */

#include <d2/event_repository.hpp>
#include <d2/events/exceptions.hpp>
#include <d2/sync_skeleton.hpp>

// Disable MSVC warning C4512: assignment operator could not be generated
#include <boost/config.hpp>
#if defined(BOOST_MSVC)
#   pragma warning(push)
#   pragma warning(disable: 4512)
#endif
#include <boost/assert.hpp>
#include <boost/assign.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/format.hpp>
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
#include <boost/spirit/include/karma.hpp>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>
#if defined(BOOST_MSVC)
#   pragma warning(pop)
#endif


static std::string const VERSION = "0.1a";

namespace fs = boost::filesystem;
namespace kma = boost::spirit::karma;
namespace phx = boost::phoenix;
namespace po = boost::program_options;

template <typename ErrorTag, typename Exception>
std::string get_error_info(Exception const& e,
                           std::string const& default_ = "\"unavailable\"") {
    typename ErrorTag::value_type const* info =
                                        boost::get_error_info<ErrorTag>(e);
    return info ? boost::lexical_cast<std::string>(*info) : default_;
}


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

    po::options_description statistics("Statistics options");

    po::options_description allowed;
    allowed.add(global).add(analysis).add(statistics);

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
        std::cerr << "missing the path of a repository on which to operate\n"
                  << allowed << std::endl;
        return EXIT_FAILURE;
    }
    fs::path repo_path = args["repo-path"].as<std::string>();
    if (!fs::exists(repo_path)) {
        std::cerr << boost::format("the repository path %1% does not exist\n")
                                                                % repo_path;
        return EXIT_FAILURE;
    }

    typedef d2::EventRepository<> Repository;
    boost::scoped_ptr<Repository> repository;
    try {
        repository.reset(new Repository(repo_path));
    } catch (d2::RepositoryException const& e) {
        std::cerr << boost::format("unable to open the repository at %1%\n")
                                                                % repo_path;
        if (args.count("debug"))
            std::cerr << boost::diagnostic_information(e) << '\n';
        return EXIT_FAILURE;
    }
    BOOST_ASSERT(repository);

    // Open the output stream to whatever passed on the command line or to
    // stdout if unspecified.
    std::ofstream output_ofs;
    if (args.count("output-file")) {
        std::string output_file = args["output-file"].as<std::string>();
        output_ofs.open(output_file.c_str());
        if (!output_ofs) {
            std::cerr << boost::format("unable to open output file \"%1%\"\n")
                                                                % output_file;
            return EXIT_FAILURE;
        }
    }
    std::ostream& output = args.count("output-file") ? output_ofs : std::cout;

    // Create the skeleton of the program from the repository.
    typedef d2::SyncSkeleton<Repository> Skeleton;
    boost::scoped_ptr<Skeleton> skeleton;
    try {
        skeleton.reset(new Skeleton(*repository));
    } catch (d2::EventTypeException const& e) {
        std::string actual_type = get_error_info<d2::ActualType>(e);
        std::string expected_type = get_error_info<d2::ExpectedType>(e);

        std::cerr << boost::format(
        "error while building the graphs:\n"
        "    encountered an event of type %1%\n"
        "    while expecting an event of type %2%\n")
        % actual_type % expected_type;
        return EXIT_FAILURE;

    } catch (d2::UnexpectedReleaseException const& e) {
        std::string lock = get_error_info<d2::ReleasedLock>(e);
        std::string thread = get_error_info<d2::ReleasingThread>(e);

        std::cerr << boost::format(
        "error while building the graphs:\n"
        "    lock %1% was unexpectedly released by thread %2%\n")
                                                            % lock % thread;
         if (args.count("debug"))
            std::cerr << boost::diagnostic_information(e) << '\n';
        return EXIT_FAILURE;
    }
    BOOST_ASSERT(skeleton);

    // Main switch dispatching to the right action to perform.
    if (args.count("analyze") || !args.count("stats")) {
        output << kma::format(
            kma::stream % ('\n' << kma::repeat(80)['-'] << '\n') << '\n'
        , skeleton->deadlocks());
    }
    else if (args.count("stats")) {
        output << boost::format(
            "number of threads: %1%\n"
            "number of distinct locks: %2%\n")
            % skeleton->number_of_threads()
            % skeleton->number_of_locks();
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
