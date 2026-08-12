#ifndef MKN_KUL_STD_INTS_HPP_
#define MKN_KUL_STD_INTS_HPP_

#include "mkn/kul/std/types.hpp"

#include <cstdint>
#include <cstddef>

namespace mkn::kul {

using Int16_t = StrongType<int16_t>;
using Int32_t = StrongType<int32_t>;
using Int64_t = StrongType<int64_t>;

using Uint16_t = StrongType<uint16_t>;
using Uint32_t = StrongType<uint32_t>;
using Uint64_t = StrongType<uint64_t>;
using Size_t = StrongType<size_t>;

}  // namespace mkn::kul

#endif /* MKN_KUL_STD_INTS_HPP_ */
