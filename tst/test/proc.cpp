

#include "test_common.hpp"

#include "mkn/kul/proc.hpp"
#include "tst/run_main.hpp"

TEST(Process_Test, LaunchProcess) {
  mkn::kul::Process p("bash");
  p << "-c"
    << "\"seq"
    << "1"
    << "10\"";
  mkn::kul::tst::run_main([&] {  // windows can fail
    p.start();
    KOUT(NON) << "Process exit code: " << p.exitCode();
    EXPECT_EQ(p.exitCode(), 0);
  });
}

TEST(Process_Test, CaptureProcess) {
  std::string lines;
  mkn::kul::Process p("bash");
  mkn::kul::ProcessCapture pc(p);
  p << "-c"
    << "\"seq"
    << "1"
    << "10\"";
  mkn::kul::tst::run_main([&] {  // windows can fail
    p.start();
    lines = pc.outs();
    ASSERT_TRUE(lines.size());
    ASSERT_FALSE(pc.errs().size());
    uint8_t i = 0;
    for (auto s : mkn::kul::String::LINES(lines)) {
      EXPECT_EQ(s, std::to_string(++i));
    }
  });
}

#if !defined(_WIN32)
// output is passed on with the length read, a NUL byte does not end it
TEST(Process_Test, CaptureKeepsNulBytes) {
  mkn::kul::Process p("head");
  mkn::kul::ProcessCapture pc(p);
  p << "-c" << "3" << "/dev/zero";
  mkn::kul::tst::run_main([&] {
    p.start();
    EXPECT_EQ(std::string(3, '\0'), pc.outs());
  });
}

// stdout and stderr each keep their order, in their own capture
TEST(Process_Test, CaptureKeepsStreamOrder) {
  mkn::kul::Process p("bash");
  mkn::kul::ProcessCapture pc(p);
  p << "-c" << "'for i in $(seq 1 2000); do echo $i; echo e$i >&2; done'";
  mkn::kul::tst::run_main([&] {
    p.start();
    std::uint16_t i = 0;
    for (auto const& s : mkn::kul::String::LINES(pc.outs())) EXPECT_EQ(s, std::to_string(++i));
    EXPECT_EQ(i, 2000);
    i = 0;
    for (auto const& s : mkn::kul::String::LINES(pc.errs()))
      EXPECT_EQ(s, "e" + std::to_string(++i));
    EXPECT_EQ(i, 2000);
  });
}
#endif
