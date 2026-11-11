

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

// a child that fills stderr before it writes to stdout: both pipes are read as they fill, a read
//  waiting on stdout alone deadlocks with the child blocked on a full stderr
TEST(Process_Test, CaptureStderrBeforeStdout) {
#if defined(_WIN32)
  mkn::kul::Process p("cmd");
  p << "/c" << "(for /L %i in (1,1,3000) do @echo e%i 1>&2) & echo done";
#else
  mkn::kul::Process p("bash");
  p << "-c" << "'for i in {1..20000}; do echo e$i >&2; done; echo done'";
#endif
  mkn::kul::ProcessCapture pc(p);
  p.start();
  EXPECT_NE(std::string::npos, pc.outs().find("done")) << pc.outs();
  auto const errs = mkn::kul::String::LINES(pc.errs());
  ASSERT_FALSE(errs.empty());
  EXPECT_EQ(0u, errs.front().find("e1")) << errs.front();
}
