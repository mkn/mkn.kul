#include <stdexcept>
#include <string_view>
#include "test/instrument/alloc.hpp"

std::string_view constexpr RUN =
    R"(
mkn -p base -M test/instrument/non_constructing_vector.cpp -Og 0 build -b . run -a -fPIC -x clang -m "clang.llvm-mca{link{bin: llvm-mca-19, out: non_vec.txt}}"
)";

template <typename T>
void go() {
  constexpr static std::size_t N = 2000000;
  auto const std_vec = make_vector<std::vector<T>>(N);
  auto const std_vec2 = make_vector_from<mkn::kul::NonConstructingVector<T>>(std_vec);
  if (std_vec2[0][0] != 0) throw std::runtime_error("do not optimize out");
}

int main() { go<S<8>>(); }
