
#include <d2/detail/config.hpp>
#ifdef D2_WIN32
// Disable MSVC C4996: Function call with parameters that may be unsafe.
#   define _SCL_SECURE_NO_WARNINGS
#endif

#include "mock.hpp"

#include <algorithm>
#include <boost/move/move.hpp>
#include <boost/range/adaptors.hpp>
#include <boost/range/algorithm.hpp>
#include <cstddef>
#include <vector>


static std::size_t const NOISE_THREADS = 10;
static std::size_t const MUTEXES_PER_NOISE_THREAD = 100;

int main(int argc, char const* argv[]) {
    auto noise = [&] {
        std::vector<d2::mock::mutex> mutexes(MUTEXES_PER_NOISE_THREAD);
        boost::for_each(mutexes, [](d2::mock::mutex& m) { m.lock(); });
        boost::for_each(mutexes | boost::adaptors::reversed,
                                    [](d2::mock::mutex& m) { m.unlock(); });
    };

    d2::mock::mutex A, B;

    d2::mock::thread t0([&] {
        A.lock();
            B.lock();
            B.unlock();
        A.unlock();
    });

    d2::mock::thread t1([&] {
        B.lock();
            A.lock();
            A.unlock();
        B.unlock();
    });

    std::vector<d2::mock::thread> threads;
    threads.push_back(boost::move(t0)); threads.push_back(boost::move(t1));
    std::generate_n(boost::back_move_inserter(threads), NOISE_THREADS, [&] {
        return d2::mock::thread(noise);
    });
    boost::range::random_shuffle(threads);


    d2::mock::integration_test start(argc, argv, __FILE__);

    boost::for_each(threads, [](d2::mock::thread& t) { t.start(); });
    boost::for_each(threads, [](d2::mock::thread& t) { t.join(); });
}
