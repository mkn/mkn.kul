#ifndef MKN_KUL_STD_STRING_HPP_
#define MKN_KUL_STD_STRING_HPP_

#include <sstream>
#include <string>

namespace mkn::kul {

inline std::string to_str(auto&&... args) {
  std::ostringstream ss;
  (ss << ... << args);
  return ss.str();
}

}  // namespace mkn::kul

#endif /* MKN_KUL_STD_STRING_HPP_ */
