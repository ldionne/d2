/*!
 * @file
 * This file implements the d2/trackable_sync_object.hpp header.
 */

#define D2_SOURCE
#include <d2/detail/decl.hpp>
#include <d2/trackable_sync_object.hpp>

#include <boost/functional/hash.hpp>
#include <boost/thread/thread.hpp>
#include <cstddef>


namespace d2 { namespace trackable_sync_object_detail {
    D2_DECL extern std::size_t this_thread_id() {
        /*!
         * @todo
         * Find a better way to do this portably, or at least
         * trigger an error when it would yield invalid results.
         */
        using boost::hash_value;
        return hash_value(boost::this_thread::get_id());
    }
}}
