/**
 * This file contains compile-time tests for the `EventDispatcher` class.
 */

#include <d2/event_dispatcher.hpp>


namespace {

struct DummyOstream { };
static DummyOstream dummy_ostream;

// Make sure the types associated to each event scope is the same as generated
// by the policy.
struct TestSinkContainerTypePolicy {
    template <typename Scope>
    struct apply {
        struct type { };
    };

    // Will only work if sink containers have the right type.
    template <typename Event, typename Scope>
    static DummyOstream& get_sink(typename apply<Scope>::type, Event const&,
                                                               Scope) {
        return dummy_ostream;
    }
};

struct TestEvent {
    typedef d2::thread_scope event_scope;
    typedef d2::strict_order_policy ordering_policy;

    template <typename Ostream>
    friend Ostream& operator<<(Ostream& os, TestEvent const&) {
        return os;
    }
};

template <typename Policy>
void instantiate() {
    d2::EventDispatcher<TestSinkContainerTypePolicy> dispatcher;
    TestEvent e;
    dispatcher.dispatch(e);
}

} // end anonymous namespace

int main() {
    instantiate<TestSinkContainerTypePolicy>();
}
