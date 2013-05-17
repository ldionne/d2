/**
 * This file implements the `d2` raw API.
 */

#define D2_SOURCE
#include <d2/core/framework.hpp>
#include <d2/core/raw_api.hpp>
#include <d2/detail/decl.hpp>


namespace d2 {
namespace core {
namespace raw_api_detail {
    D2_DECL extern framework& get_framework() {
        static framework FRAMEWORK;
        return FRAMEWORK;
    }
}
}
}
