

#include "test_common.hpp"

#include "mkn/kul/except.hpp"

TEST(Exception, String) {
  std::string full = "FAILURE";
  try {
    KEXCEPT(mkn::kul::Exception, "FAIL") << "URE";
  } catch (std::exception const& e) {
    EXPECT_EQ(e.what(), full);
  }
}
