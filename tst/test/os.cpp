

#include "test_common.hpp"

#include "mkn/kul/os.hpp"
#include "mkn/kul/proc.hpp"
#include "mkn/kul/mem.hpp"

#include <filesystem>
#include <fstream>
#include <vector>

TEST(OperatingSystemTests, HasRAMUsageSupport) {
  ASSERT_TRUE(mkn::kul::this_proc::physicalMemory());
  ASSERT_TRUE(mkn::kul::this_proc::virtualMemory());
}

TEST(OperatingSystemTests, MemBytesSupport) {
  namespace mem = mkn::kul::mem;
  auto const rss = mem::rss_bytes();
  ASSERT_GT(rss, 0u);
  ASSERT_GT(mem::virtual_bytes(), 0u);
  ASSERT_GE(mem::peak_rss_bytes(), rss);
  auto const u = mem::usage();
  ASSERT_GT(u.rss, 0u);
  ASSERT_GE(u.peak, u.rss);
}

TEST(OperatingSystemTests, MemRSSGrowsWhenTouched) {
  namespace mem = mkn::kul::mem;
  std::size_t constexpr N = 64 * 1024 * 1024;
  auto const before = mem::rss_bytes();
  std::vector<char> v(N, 1);  // value-init touches every page
  auto const after = mem::rss_bytes();
  ASSERT_GE(after, before + N / 2);
  ASSERT_GE(mem::peak_rss_bytes(), after);
  ASSERT_EQ(v[N - 1], 1);
}

class TimeStampHandler {
 private:
  mkn::kul::File f;
  mkn::kul::fsys::TimeStamps fts;
  TimeStampHandler() : f("mkn.yaml"), fts(f.timeStamps()) {}

 public:
  static TimeStampHandler& INSTANCE() {
    static TimeStampHandler i;
    return i;
  }
  bool is() { return f.is(); }
  mkn::kul::fsys::TimeStamps const& timeStamps() { return fts; }
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

TEST(OperatingSystemTests, DirMkFilesDirsCpMvRm) {
  using mkn::kul::Dir, mkn::kul::File;
  Dir const tmp("bin/test/kul_os_fs_test");
  tmp.rm();
  ASSERT_FALSE(tmp.is());

  Dir const sub(tmp.join("a/b"));
  ASSERT_TRUE(sub.mk());   // creates parents
  EXPECT_FALSE(sub.mk());  // already exists
  ASSERT_TRUE(tmp.is());
  Dir const hidden(tmp.join(".hidden"));
  ASSERT_TRUE(hidden.mk());

  File const f("f.txt", tmp);
  ASSERT_TRUE(f.mk());
  ASSERT_TRUE(f.is());
  EXPECT_EQ(f.size(), 0u);
  {
    std::ofstream(f.full()) << "12345";
  }
  EXPECT_EQ(f.size(), 5u);
  EXPECT_FALSE(Dir(f.full()).is());
  EXPECT_FALSE(File("a", tmp).is());

  EXPECT_EQ(tmp.files().size(), 1u);
  EXPECT_EQ(tmp.dirs().size(), 1u);
  EXPECT_EQ(tmp.dirs(true).size(), 2u);

  ASSERT_TRUE(f.cp(sub));
  File const copied("f.txt", sub);
  ASSERT_TRUE(copied.is());
  EXPECT_EQ(copied.size(), 5u);
  EXPECT_EQ(tmp.files(true).size(), 2u);

  File const moved("g.txt", tmp);
  ASSERT_TRUE(f.mv(moved));
  EXPECT_FALSE(f.is());
  EXPECT_TRUE(moved.is());

  EXPECT_TRUE(moved.rm());
  EXPECT_FALSE(moved.rm());

  tmp.rm();  // removes hidden directories too
  EXPECT_FALSE(tmp.is());
}

TEST(OperatingSystemTests, DirRealAndRoot) {
  using mkn::kul::Dir;
  EXPECT_FALSE(Dir("inc").root());
  EXPECT_EQ(Dir("inc/mkn/..").real(), Dir("inc").real());
  EXPECT_EQ(Dir("inc").real(), Dir::JOIN(Dir(mkn::kul::env::CWD()).real(), "inc"));
#ifndef _WIN32
  EXPECT_TRUE(Dir("/").root());
  EXPECT_FALSE(Dir::REAL_OR_NULL("does_not_exist_kul_os_test"));
  EXPECT_THROW(Dir::REAL("does_not_exist_kul_os_test"), mkn::kul::fsys::Exception);
#endif
}

TEST(OperatingSystemTests, FsysFilesDirs) {
  namespace fsys = mkn::kul::fsys;
  using path = std::filesystem::path;
  path const tmp("bin/test/kul_fsys_test");
  std::filesystem::remove_all(tmp);
  for (auto const* d : {"b/c", "a", ".hidden"}) std::filesystem::create_directories(tmp / d);
  for (auto const* f : {"z.txt", ".dotfile", "b/y.txt", "b/c/x.txt", ".hidden/w.txt"})
    std::ofstream(tmp / f) << "";

  EXPECT_EQ(fsys::dirs(tmp), (std::vector<path>{tmp / "a", tmp / "b"}));
  EXPECT_EQ(fsys::dirs(tmp, {.hidden = true}),
            (std::vector<path>{tmp / ".hidden", tmp / "a", tmp / "b"}));
  EXPECT_EQ(fsys::dirs(tmp, {.recursive = true}),
            (std::vector<path>{tmp / "a", tmp / "b", tmp / "b/c"}));

  EXPECT_EQ(fsys::files(tmp), (std::vector<path>{tmp / ".dotfile", tmp / "z.txt"}));
  EXPECT_EQ(
      fsys::files(tmp, {.recursive = true}),
      (std::vector<path>{tmp / ".dotfile", tmp / "b/c/x.txt", tmp / "b/y.txt", tmp / "z.txt"}));
  EXPECT_EQ(fsys::files(tmp, {.recursive = true, .hidden = true}).size(), 5u);

  EXPECT_THROW((void)fsys::files(tmp / "nope"), fsys::Exception);
  EXPECT_THROW((void)fsys::dirs(tmp / "z.txt"), fsys::Exception);

  std::filesystem::remove_all(tmp);
}

// the fsys free functions give the same strings as the File / Dir members they replace
TEST(OperatingSystemTests, FsysMatchesFileDir) {
  namespace fsys = mkn::kul::fsys;
  using mkn::kul::Dir, mkn::kul::File;
  Dir const tmp("bin/test/kul fsys mini");  // a space, for esc
  tmp.rm();
  ASSERT_TRUE(tmp.mk());
  File const f("a file.txt", tmp);
  ASSERT_TRUE(f.mk());

  for (auto const& p : {std::string{"mkn.yaml"}, std::string{"inc/../mkn.yaml"}, f.full(),
                        File("missing.txt", tmp).full()}) {
    File const file(p);
    EXPECT_EQ(fsys::real(p), file.real()) << p;
    EXPECT_EQ(fsys::mini(p), file.mini()) << p;
    EXPECT_EQ(fsys::esc(file.full()), file.esc()) << p;
    EXPECT_EQ(fsys::escm(p), file.escm()) << p;
  }
  for (auto const& p :
       {std::string{"inc"}, std::string{"inc/mkn/.."}, tmp.path(), mkn::kul::env::CWD()}) {
    Dir const dir(p);
    EXPECT_EQ(fsys::real(p), dir.real()) << p;
    EXPECT_EQ(fsys::mini(p), dir.mini()) << p;
    EXPECT_EQ(fsys::esc(p), dir.esc()) << p;
    EXPECT_EQ(fsys::escm(p), dir.escm()) << p;
  }
  EXPECT_EQ(fsys::modified("mkn.yaml"), File("mkn.yaml").timeStamps().modified());
  EXPECT_GT(fsys::modified("mkn.yaml"), 0u);
  EXPECT_EQ(fsys::modified("does_not_exist_kul_os_test"), 0u);

  {
    auto const inc = Dir("inc").real();
    mkn::kul::os::PushDir const pd(std::filesystem::path{"inc"});
    EXPECT_EQ(std::string{mkn::kul::env::CWD()}, inc);
  }
  EXPECT_THROW(mkn::kul::os::PushDir{"does_not_exist_kul_os_test"}, mkn::kul::Exception);
  tmp.rm();
}
