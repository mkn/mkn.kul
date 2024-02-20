
#ifndef _MKN_KUL_TESTS_CMN_HPP_
#define _MKN_KUL_TESTS_CMN_HPP_

#include "gtest/gtest.h"

#include <algorithm>

#include "mkn/kul/log.hpp"
#include "mkn/kul/except.hpp"


template <typename T, std::size_t N>
void bzero(T(&ar)[N]) {
  std::fill_n(&ar[0], N, 0);
}

void inline tryCatch(std::vector<std::function<void()>> funcs, bool katch) {
  for (auto const& func : funcs) try {
      func();
      ASSERT_TRUE(!katch);
    } catch (mkn::kul::Exception const& e) {
      if (!katch) KOUT(NON) << e.debug();
      ASSERT_TRUE(katch);
    }
};

#endif /*_MKN_KUL_TESTS_CMN_HPP_*/
