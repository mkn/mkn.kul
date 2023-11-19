

TEST(Process_Test, LaunchProcess) {
  mkn::kul::Process p("bash");
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
  mkn::kul::Process p("bash");
  mkn::kul::ProcessCapture pc(p);
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
    for (auto s : mkn::kul::String::LINES(lines)) {
      EXPECT_EQ(s, std::to_string(++i));
    }
  } catch (...) {
  }
}
