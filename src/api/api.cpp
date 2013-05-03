/**
 * This file implements the _d2_ API.
 */

#define D2_SOURCE
#include <d2/api.h>
#include <d2/core/framework.hpp>
#include <d2/detail/decl.hpp>

#include <stddef.h>


namespace d2 {
    namespace {
        core::framework& get_framework() {
            static core::framework FRAMEWORK;
            return FRAMEWORK;
        }
    }
}

D2_DECL extern void d2_disable_event_logging(void) {
    d2::get_framework().disable();
}

D2_DECL extern void d2_enable_event_logging(void) {
    d2::get_framework().enable();
}

D2_DECL extern int d2_is_enabled(void) {
    return d2::get_framework().is_enabled() ? 1 : 0;
}

D2_DECL extern int d2_is_disabled(void) {
    return d2_is_enabled() ? 0 : 1;
}

D2_DECL extern int d2_set_log_repository(char const* path) {
    return d2::get_framework().set_repository(path);
}

D2_DECL extern void d2_unset_log_repository(void) {
    d2::get_framework().unset_repository();
}

D2_DECL extern void d2_notify_acquire(size_t thread_id, size_t lock_id) {
    d2::get_framework().notify_acquire(thread_id, lock_id);
}

D2_DECL extern void d2_notify_recursive_acquire(size_t thread_id,
                                               size_t lock_id) {
    d2::get_framework().notify_recursive_acquire(thread_id, lock_id);
}

D2_DECL extern void d2_notify_release(size_t thread_id, size_t lock_id) {
    d2::get_framework().notify_release(thread_id, lock_id);
}

D2_DECL extern void d2_notify_recursive_release(size_t thread_id,
                                               size_t lock_id) {
    d2::get_framework().notify_recursive_release(thread_id, lock_id);
}

D2_DECL extern void d2_notify_start(size_t parent_id, size_t child_id) {
    d2::get_framework().notify_start(parent_id, child_id);
}

D2_DECL extern void d2_notify_join(size_t parent_id, size_t child_id) {
    d2::get_framework().notify_join(parent_id, child_id);
}
