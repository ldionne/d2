/**
 * Detect the simplest acquisition order inconsistency between two threads
 * and two locks, but with several unrelated threads and locks making noise.
 */

#include <d2mock.hpp>

#include <algorithm>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/shared_ptr.hpp>
#include <cstddef>
#include <iterator>
#include <vector>


static std::size_t const NOISE_THREADS = 10;
static std::size_t const MUTEXES_PER_NOISE_THREAD = 15;
typedef boost::shared_ptr<d2mock::thread> ThreadPtr;

int main(int argc, char const* argv[]) {
    d2mock::mutex A, B;

    auto A_or_B = [&] () -> d2mock::mutex& {
        d2mock::mutex* dataset[2] = {&A, &B};
        boost::random_shuffle(dataset);
        return *dataset[0];
    };

    auto noise = [&] {
        std::vector<d2mock::mutex> local_mutexes(MUTEXES_PER_NOISE_THREAD);
        std::vector<d2mock::mutex*> mutexes;
        for (auto& m: local_mutexes)
            mutexes.push_back(&m);
        mutexes.push_back(&A_or_B());
        boost::random_shuffle(mutexes);

        boost::for_each(mutexes, [](d2mock::mutex* m) { m->lock(); });
        boost::for_each(mutexes | boost::adaptors::reversed,
                                    [](d2mock::mutex* m) { m->unlock(); });
    };

    ThreadPtr t0(new d2mock::thread([&] {
        A.lock();
            B.lock();
            B.unlock();
        A.unlock();
    }));

    ThreadPtr t1(new d2mock::thread([&] {
        B.lock();
            A.lock();
            A.unlock();
        B.unlock();
    }));

    std::vector<ThreadPtr> threads;
    threads.push_back(t0); threads.push_back(t1);
    std::generate_n(std::back_inserter(threads), NOISE_THREADS, [&] {
        return ThreadPtr(new d2mock::thread(noise));
    });
    boost::random_shuffle(threads);

    auto test_main = [&] {
        boost::for_each(threads, [](ThreadPtr t) { t->start(); });
        boost::for_each(threads, [](ThreadPtr t) { t->join(); });
    };

    return d2mock::check_scenario(test_main, argc, argv, {
                {
                    // t0 holds A, and waits for B
                    {*t0, A, B},
                    // t1 holds B, and waits for A
                    {*t1, B, A}
                }
            });
}
