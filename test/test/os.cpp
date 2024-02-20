

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
