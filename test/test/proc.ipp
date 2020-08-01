

TEST(Process_Test, LaunchProcess) {
  kul::Process p("bash");
  p << "-c"
    << "\"seq"
    << "1"
    << "10\"";
  try {  // windows can fail
    p.start();
    KOUT(NON) << "Process exit code: " << p.exitCode();
    EXPECT_EQ(p.exitCode(), 0);
  } catch (...) {
  }
}

TEST(Process_Test, CaptureProcess) {
  std::string lines;
  kul::Process p("bash");
  kul::ProcessCapture pc(p);
  p << "-c"
    << "\"seq"
    << "1"
    << "10\"";
  try {  // windows can fail
    p.start();
    lines = pc.outs();
    ASSERT_TRUE(lines.size());
    ASSERT_FALSE(pc.errs().size());
    uint8_t i = 0;
    for (auto s : kul::String::LINES(lines)) {
      EXPECT_EQ(s, std::to_string(++i));
    }
  } catch (...) {
  }
}
