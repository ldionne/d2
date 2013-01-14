/**
 * Integration test to try using the library from C.
 */

#include <d2/detail/config.hpp>
#ifdef D2_WIN32
/* Disable MSVC C4996: This function or variable may be unsafe. (tmpnam) */
#   define _CRT_SECURE_NO_WARNINGS
#endif

#include <d2/logging.h>

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>


int main(int argc, char const* argv[]) {
    size_t A, B, t0, t1;
    char repository[L_tmpnam];
    (void)argc; (void)argv;
    A = 4;
    B = 8;
    t0 = 0;
    t1 = 1;

    tmpnam(repository);
    if (d2_set_log_repository(repository)) {
        fprintf(stderr,
            "unable to set repository to \"%s\", aborting\n", repository);
        return EXIT_FAILURE;
    }
    fprintf(stdout, "repository is at \"%s\"\n", repository);
    assert(d2_is_disabled());

    d2_enable_event_logging();
    assert(d2_is_enabled());

    d2_notify_start(t0, t1);
        d2_notify_acquire(t0, A);
            d2_notify_acquire(t0, B);
            d2_notify_release(t0, B);
        d2_notify_release(t0, A);

        d2_notify_acquire(t1, B);
            d2_notify_acquire(t1, A);
            d2_notify_release(t1, A);
        d2_notify_release(t1, B);
    d2_notify_join(t0, t1);

    d2_disable_event_logging();
    assert(d2_is_disabled());

    return EXIT_SUCCESS;
}
