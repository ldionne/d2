/**
 * This file implements a command line utility to interact
 * with the `d2` library.
 */

#include <boost/exception/diagnostic_information.hpp>
#include <boost/exception/get_error_info.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>
#include <cstdlib> // EXIT_FAILURE, EXIT_SUCCESS
#include <d2/core/diagnostic.hpp>
#include <d2/core/exceptions.hpp>
#include <d2/core/filesystem.hpp>
#include <d2/core/synchronization_skeleton.hpp>
#include <exception>
#include <iostream>
#include <string>


namespace {
namespace po = boost::program_options;

class Driver {
    /**
     * Return a string representation of the data associated to an `ErrorTag`.
     *
     * If no data associated to that tag is present in the exception object,
     * `default_` is returned instead.
     */
    template <typename ErrorTag, typename Exception>
    static std::string
    get_error_info(Exception const& e, std::string default_ = "unavailable") {
        typedef typename ErrorTag::value_type Info;
        Info const* info = boost::get_error_info<ErrorTag>(e);
        return info ? boost::lexical_cast<std::string>(*info) : default_;
    }

    typedef d2::core::synchronization_skeleton Skeleton;

    //! Print an error to the standard error stream with a trailing newline.
    template <typename Printable>
    static void error(Printable const& p) {
        std::cerr << "d2: error: " << p << '\n';
    }

    /**
     * Return a pointer to a `synchronization_skeleton` loaded with the data
     * found in the repository, or a default initialized `shared_ptr` if
     * anything goes wrong.
     */
    boost::shared_ptr<Skeleton> create_skeleton() const {
        if (!boost::filesystem::exists(repo)) {
            error(repo + " does not exist");
            return boost::shared_ptr<Skeleton>();
        }

        try {
            return boost::make_shared<Skeleton>(repo);

        } catch (d2::core::filesystem_error const& e) {
            error("unable to open the repository at " + repo);
            if (debug)
                error(boost::diagnostic_information(e));

        } catch (d2::EventTypeException const& e) {
            std::string actual_type = get_error_info<d2::ActualType>(e);
            std::string expected_type = get_error_info<d2::ExpectedType>(e);

            error(boost::format(
                    "while building the graphs:\n"
                    "    encountered an event of type %1%\n"
                    "    while expecting an event of type %2%")
                    % actual_type % expected_type);
            if (debug)
                error(boost::diagnostic_information(e));

        } catch (d2::UnexpectedReleaseException const& e) {
            std::string lock = get_error_info<d2::ReleasedLock>(e);
            std::string thread = get_error_info<d2::ReleasingThread>(e);

            error(boost::format(
                    "while building the graphs:\n"
                    "    lock %1% was unexpectedly released by thread %2%")
                    % lock % thread);

             if (debug)
                error(boost::diagnostic_information(e));
        }
        return boost::shared_ptr<Skeleton>();
    }

    bool parse_command_line(int argc, char const* argv[]) {
        po::variables_map args;
        po::options_description visible, hidden, all;
        po::positional_options_description positionals;
        bool help = false;

        visible.add_options()
        (
            "help,h",
            po::bool_switch(&help),
            "produce help message and exit"
        )(
            "analyze",
            po::bool_switch(&analyze)->default_value(true, "true"),
            "perform the analysis for deadlocks"
        )(
            "stats",
            po::bool_switch(&stats),
            "produce statistics about the usage of locks and threads"
        )(
            "debug",
            po::bool_switch(&debug),
            "enable special debugging output"
        )
        ;

        hidden.add_options()
        (
            "repo-path",
            po::value<std::string>(&repo),
            "path of the repository to examine"
        )
        ;

        positionals.add("repo-path", 1);
        all.add(visible).add(hidden);

        po::command_line_parser parser(argc, argv);
        try {
            po::store(parser.options(all).positional(positionals).run(), args);
        } catch (po::error const& e) {
            error(e.what());
            return false;
        }
        po::notify(args);

        if (help) {
            std::cout << visible;
            return false;
        }

        if (repo.empty()) {
            error("missing input directory");
            return false;
        }

        return true;
    }

    std::string repo;
    bool debug, analyze, stats;

    static void print_deadlock(d2::core::potential_deadlock const& dl) {
        std::cout <<
        // 80 columns
"\n--------------------------------------------------------------------------------\n";
        d2::core::plain_text_explanation(std::cout, dl);
        std::cout << '\n';
    }

public:
    int run(int argc, char const* argv[]) {
        try {
            if (!parse_command_line(argc, argv))
                return EXIT_FAILURE;

            boost::shared_ptr<Skeleton> skeleton = create_skeleton();
            if (!skeleton)
                return EXIT_FAILURE;

            if (analyze)
                skeleton->on_deadlocks(print_deadlock);

            if (stats) {
                std::cout << boost::format(
                    "number of threads: %1%\n"
                    "number of distinct locks: %2%\n")
                    % skeleton->number_of_threads()
                    % skeleton->number_of_locks();
            }

        } catch (std::exception const& e) {
            error("unknown problem:");
            error(boost::diagnostic_information(e));
        }

        return EXIT_SUCCESS;
    }
};
} // end anonymous namespace


int main(int argc, char const* argv[]) {
    Driver driver;
    return driver.run(argc, argv);
}
