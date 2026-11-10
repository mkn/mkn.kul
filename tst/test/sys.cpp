#include "test_common.hpp"

#include "mkn/kul/io.hpp"
#include "mkn/kul/os.hpp"
#include "mkn/kul/sys.hpp"

// callers catch sys::Exception for load failures, e.g. to skip libraries that can't be loaded
TEST(SharedLibrary, MissingFileThrowsSysException) {
  EXPECT_THROW(mkn::kul::sys::SharedLibrary{mkn::kul::File{"does_not_exist.so"}},
               mkn::kul::sys::Exception);
}

TEST(SharedLibrary, InvalidLibraryThrowsSysException) {
  mkn::kul::File const f{"not_a_library_kul_sys_test.so"};
  mkn::kul::io::Writer(f) << "not an ELF/PE file";
  EXPECT_THROW(mkn::kul::sys::SharedLibrary{f}, mkn::kul::sys::Exception);
  f.rm();
}
