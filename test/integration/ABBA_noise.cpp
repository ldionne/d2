
#include <d2/mock.hpp>

#include <algorithm>
#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/shared_ptr.hpp>
#include <cstddef>
#include <iterator>
#include <vector>


static std::size_t const NOISE_THREADS = 10;
static std::size_t const MUTEXES_PER_NOISE_THREAD = 100;
typedef boost::shared_ptr<d2::mock::thread> ThreadPtr;

int main(int argc, char const* argv[]) {
    auto noise = [&] {
        std::vector<d2::mock::mutex> mutexes(MUTEXES_PER_NOISE_THREAD);
        boost::for_each(mutexes, [](d2::mock::mutex& m) { m.lock(); });
        boost::for_each(mutexes | boost::adaptors::reversed,
                                    [](d2::mock::mutex& m) { m.unlock(); });
    };

    d2::mock::mutex A, B;

    ThreadPtr t0(new d2::mock::thread([&] {
        A.lock();
            B.lock();
            B.unlock();
        A.unlock();
    }));

    ThreadPtr t1(new d2::mock::thread([&] {
        B.lock();
            A.lock();
            A.unlock();
        B.unlock();
    }));

    std::vector<ThreadPtr> threads;
    threads.push_back(t0); threads.push_back(t1);
    std::generate_n(std::back_inserter(threads), NOISE_THREADS, [&] {
        return ThreadPtr(new d2::mock::thread(noise));
    });
    boost::random_shuffle(threads);


    d2::mock::integration_test integration_test(argc, argv, __FILE__);

    boost::for_each(threads, [](ThreadPtr t) { t->start(); });
    boost::for_each(threads, [](ThreadPtr t) { t->join(); });

    integration_test.verify_deadlocks({
        {
            {*t0, A, B},
            {*t1, B, A}
        }
    });
}
