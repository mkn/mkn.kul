

#include "test_common.hpp"

#include "mkn/kul/os.hpp"
#include "mkn/kul/proc.hpp"

TEST(OperatingSystemTests, HasRAMUsageSupport) {
  ASSERT_TRUE(mkn::kul::this_proc::physicalMemory());
  ASSERT_TRUE(mkn::kul::this_proc::virtualMemory());
}

class TimeStampHandler {
 private:
  mkn::kul::File f;
  mkn::kul::fs::TimeStamps fts;
  TimeStampHandler() : f("mkn.yaml"), fts(f.timeStamps()) {}

 public:
  static TimeStampHandler& INSTANCE() {
    static TimeStampHandler i;
    return i;
  }
  bool is() { return f.is(); }
  mkn::kul::fs::TimeStamps const& timeStamps() { return fts; }
};

TEST(OperatingSystemTests, HasFileAccessedTimeStampSupport) {
  ASSERT_TRUE(TimeStampHandler::INSTANCE().is());
  ASSERT_TRUE(TimeStampHandler::INSTANCE().timeStamps().accessed());
}
#ifdef _WIN32
TEST(OperatingSystemTests, HasFileCreatedTimeStampSupport) {
  ASSERT_TRUE(TimeStampHandler::INSTANCE().is());
  ASSERT_TRUE(TimeStampHandler::INSTANCE().timeStamps().created());
}
#endif
TEST(OperatingSystemTests, HasFileModifiedTimeStampSupport) {
  ASSERT_TRUE(TimeStampHandler::INSTANCE().is());
  ASSERT_TRUE(TimeStampHandler::INSTANCE().timeStamps().modified());
}

TEST(OperatingSystemTests, FileAbsolutePathMatchesRelativeForTopLevelFile) {
  mkn::kul::File const relative("mkn.yaml");
  ASSERT_TRUE(relative.is());
  mkn::kul::File const absolute(relative.real());
  EXPECT_TRUE(absolute.is());
  EXPECT_EQ(relative.name(), absolute.name());
  EXPECT_EQ(relative.real(), absolute.real());
  EXPECT_EQ(relative.dir().real(), absolute.dir().real());
  EXPECT_EQ(relative.size(), absolute.size());
}

TEST(OperatingSystemTests, FileAbsolutePathMatchesRelativeForNestedFile) {
  mkn::kul::File const relative("inc/mkn/kul/os.hpp");
  ASSERT_TRUE(relative.is());
  mkn::kul::File const absolute(relative.real());
  EXPECT_TRUE(absolute.is());
  EXPECT_EQ(relative.name(), absolute.name());
  EXPECT_EQ(relative.real(), absolute.real());
  EXPECT_EQ(relative.dir().real(), absolute.dir().real());
  EXPECT_EQ(relative.size(), absolute.size());
}

TEST(OperatingSystemTests, FileAbsolutePathResolvesRegardlessOfCWD) {
  mkn::kul::File const relative("LICENSE.md");
  ASSERT_TRUE(relative.is());
  std::string const absPath = relative.real();

  mkn::kul::os::PushDir const pd(mkn::kul::Dir("inc"));
  mkn::kul::File const absolute(absPath);
  EXPECT_TRUE(absolute.is());
  EXPECT_EQ(absPath, absolute.real());
}

TEST(OperatingSystemTests, FileAbsolutePathIsCanonicalized) {
  std::string const messy =
      mkn::kul::env::CWD() + mkn::kul::Dir::SEP() + mkn::kul::Dir::SEP() + "mkn.yaml";
  mkn::kul::File const f(messy);
  ASSERT_TRUE(f.is());
  EXPECT_EQ("mkn.yaml", f.name());
  EXPECT_EQ(mkn::kul::Dir(mkn::kul::env::CWD()).path(), f.dir().path());
}

TEST(OperatingSystemTests, FileAbsoluteNonExistentPathSplitsDirAndName) {
  std::string const absPath =
      mkn::kul::env::CWD() + mkn::kul::Dir::SEP() + "does_not_exist_kul_os_test.txt";
  mkn::kul::File const f(absPath);
  EXPECT_FALSE(f.is());
  EXPECT_EQ("does_not_exist_kul_os_test.txt", f.name());
  EXPECT_EQ(mkn::kul::env::CWD(), f.dir().path());
}

TEST(OperatingSystemTests, FileBareNonExistentNameDefaultsToCWD) {
  mkn::kul::File const f("does_not_exist_kul_os_test.txt");
  EXPECT_FALSE(f.is());
  EXPECT_EQ(mkn::kul::Dir(mkn::kul::env::CWD()).real(), f.dir().real());
}
