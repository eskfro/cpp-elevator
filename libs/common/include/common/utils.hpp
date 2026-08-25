#pragma once

#include "common/types.hpp"
#include <string_view>

namespace elev::common {

inline void Abort(std::string_view s) {
    PrintError(s);
    abort();
}

} // namespace elev::common