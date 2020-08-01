

TEST(OperatingSystemTests, HasRAMUsageSupport) {
  ASSERT_TRUE(kul::this_proc::physicalMemory());
  ASSERT_TRUE(kul::this_proc::virtualMemory());
}

class TimeStampHandler {
 private:
  kul::File f;
  kul::fs::TimeStamps fts;
  TimeStampHandler() : f("mkn.yaml"), fts(f.timeStamps()) {}

 public:
  static TimeStampHandler& INSTANCE() {
    static TimeStampHandler i;
    return i;
  }
  bool is() { return f.is(); }
  const kul::fs::TimeStamps& timeStamps() { return fts; }
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
