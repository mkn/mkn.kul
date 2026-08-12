
#ifndef MKN_KUL_TST_RUN_MAIN_HPP_
#define MKN_KUL_TST_RUN_MAIN_HPP_

#include "mkn/kul/log.hpp"

#include <exception>

namespace mkn::kul::tst {

template <typename F>
int run_main(F&& f) {
  try {
    f();
  } catch (std::exception const& e) {
    KERR << e.what();
    return 1;
  } catch (...) {
    KERR << "UNKNOWN EXCEPTION CAUGHT";
    return 1;
  }
  return 0;
}

}  // namespace mkn::kul::tst

#endif /*MKN_KUL_TST_RUN_MAIN_HPP_*/
