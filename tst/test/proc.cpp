

#include "test_common.hpp"

#include "mkn/kul/proc.hpp"
#include "tst/run_main.hpp"

#include <array>
#include <csignal>
#include <string>
#include <vector>

#if defined(_WIN32)
#include <windows.h>
#include <shellapi.h>  // CommandLineToArgvW
#pragma comment(lib, "shell32.lib")
#endif  // defined(_WIN32)

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
// a child killed by a signal fails as 128 + the signal, e.g. a crash or dyld refusing to load it
TEST(Process_Test, SignalIsFailure) {
  mkn::kul::Process p("bash");
  mkn::kul::ProcessCapture pc(p);  // forked and waited for here, not through system()'s shell
  p << "-c" << "'kill -ABRT $$'";
  mkn::kul::tst::run_main([&] {
    EXPECT_THROW(p.start(), mkn::kul::proc::ExitException);
    EXPECT_EQ(p.exitCode(), 128 + SIGABRT);
  });
}

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

#if !defined(_WIN32)
// a container argv: each element one argument, quotes and spaces kept (no shell, no split)
TEST(Process_Test, ListedArgvRunsAsGiven) {
  std::array<std::string, 3> const argv{"printf", "%s|", "-DX=\"a b\""};
  mkn::kul::Process p(argv);
  mkn::kul::ProcessCapture pc(p);
  p.start();
  EXPECT_EQ(pc.outs(), "-DX=\"a b\"|");
}

TEST(Process_Test, ListedArgvFromAVectorWithoutCapture) {
  std::vector<std::string> argv{"sh", "-c", "exit 0"};
  mkn::kul::Process p(argv);
  EXPECT_NO_THROW(p.start());  // not through system(): it would split "exit 0" again
  EXPECT_EQ(p.exitCode(), 0);
}

// toString of a listed argv: a shell reads the same argv back
TEST(Process_Test, ListedArgvToStringIsShellQuoted) {
  mkn::kul::Process p(std::array{"echo", "plain", "a b", "it's", "-DX=\"y\""});
  EXPECT_EQ(p.toString(), "echo plain 'a b' 'it'\\''s' '-DX=\"y\"'");

  std::string back;
  mkn::kul::Process sh(std::array<std::string, 3>{"sh", "-c", "printf '%s|' " + [&] {
                                                    auto const s = p.toString();
                                                    return s.substr(s.find(' ') + 1);
                                                  }()});
  mkn::kul::ProcessCapture pc(sh);
  sh.start();
  EXPECT_EQ(pc.outs(), "plain|a b|it's|-DX=\"y\"|");
}
#endif  // !defined(_WIN32)

#if !defined(_WIN32)
TEST(Process_Test, ListedArgvNotWaitedFor) {
  mkn::kul::Process p(std::array{"true"}, /*wfe=*/false);
  EXPECT_NO_THROW(p.start());  // no exit code to check yet
}
#endif  // !defined(_WIN32)

// argv as a Windows command line: the C runtime's splitting rules, on every OS
TEST(Process_Test, WindowsCommandLineQuoting) {
  using mkn::kul::proc::command_line;
  EXPECT_EQ(command_line({"cl", "plain"}), "cl plain");
  EXPECT_EQ(command_line({"a b"}), "\"a b\"");
  EXPECT_EQ(command_line({""}), "\"\"");
  EXPECT_EQ(command_line({"ab\"c"}), "\"ab\\\"c\"");
  EXPECT_EQ(command_line({"a\\\\b"}), "a\\\\b");          // no quote after them: literal
  EXPECT_EQ(command_line({"a\\\"b"}), "\"a\\\\\\\"b\"");  // a\"b: 2n+1 before the quote
  EXPECT_EQ(command_line({"C:\\a dir\\"}),
            "\"C:\\a dir\\\\\"");  // doubled before the closing quote
  EXPECT_EQ(command_line({"-DX=\"a b\""}), "\"-DX=\\\"a b\\\"\"");
}

#if defined(_WIN32)
// the same argvs read back by Windows' own splitter
TEST(Process_Test, WindowsCommandLineRoundTrips) {
  std::vector<std::string> const argv{"prog",   "plain",  "a b",         "",           "ab\"c",
                                      "a\\\\b", "a\\\"b", "C:\\a dir\\", "-DX=\"a b\""};
  auto const line = mkn::kul::proc::command_line(argv);
  std::wstring const wide(line.begin(), line.end());  // ASCII
  int n = 0;
  LPWSTR* const back = CommandLineToArgvW(wide.c_str(), &n);
  ASSERT_NE(back, nullptr);
  ASSERT_EQ(static_cast<std::size_t>(n), argv.size()) << line;
  for (int i = 0; i < n; ++i) {
    std::wstring const w(back[i]);
    EXPECT_EQ(std::string(w.begin(), w.end()), argv[static_cast<std::size_t>(i)]) << line;
  }
  LocalFree(back);
}

TEST(Process_Test, ListedArgvRunsOnWindows) {
  mkn::kul::Process p(std::array{"cmd", "/c", "exit 0"});
  EXPECT_NO_THROW(p.start());
  EXPECT_EQ(p.exitCode(), 0);
}
#endif  // defined(_WIN32)
