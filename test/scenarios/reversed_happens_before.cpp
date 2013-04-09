/*!
 * @file
 * This test checks whether we detect a false positive if the threads are
 * synchronized by start()/join() relationships but the cycle happens to
 * never have an edge e1 followed by an edge e2 where e1.segment2 hapens
 * before e2.segment1. Instead, edges can either happen in parallel or
 * e2.segment2 happens before e1.segment1.
 *
 * This is specifically intended to make sure the `happens_before` checked
 * during the analysis is strict enough. This test was motivated by a feeling
 * that checking whether `happens_before(edge1.segment2, edge2.segment1)` was
 * not enough, and that `happens_before(edge2.segment2, edge1.segment1)`
 * should also be checked. Currently, this is not the case because we check
 * the conditions for a deadlock for all pairs of edges in the cycle, which
 * makes us check all the conditions for (e1, e2) and (e2, e1). While we
 * redundantly check for gatelocks intersection and thread equality, we also
 * check non-redundantly for the happens-before relationship, which ensures
 * the correctness of the algorithm.
 *
 * @todo This test should be a unit test of the analysis algorithm.
 */

#include <d2mock.hpp>


int main(int argc, char const* argv[]) {
    // The order of creation is important; it makes it more likely that
    // A will appear first in the list of vertices of the lock graph,
    // making the algorithm find a cycle A->B->C->A instead of some
    // other permutation of it. If the algorithm finds another permutation
    // of the cycle, this test is useless.
    d2mock::mutex A;
    d2mock::mutex B;
    d2mock::mutex C;

    d2mock::thread t0([&] {
        A.lock();
            B.lock();
            B.unlock();
        A.unlock();
    });

    d2mock::thread t1([&] {
        B.lock();
            C.lock();
            C.unlock();
        B.unlock();
    });

    d2mock::thread t2([&] {
        C.lock();
            A.lock();
            A.unlock();
        C.unlock();
    });

    auto test_main = [&] {
        t0.start();
            t2.start();
            t2.join();

            t1.start();
            t1.join();
        t0.join();
    };

    return d2mock::check_scenario(test_main, argc, argv, {/* none */});
}
