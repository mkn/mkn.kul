#include "test_common.hpp"

#include "mkn/kul/proc.hpp"
#include "mkn/kul/scm.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#if !defined(_WIN32)  // file:// clones and bash
#include <unistd.h>   // getpid

namespace {

namespace fs = std::filesystem;

// git in dir, its output
std::string git(fs::path const& dir, std::vector<std::string> const& args) {
  mkn::kul::Process p("git", dir.string());
  mkn::kul::ProcessCapture pc(p);
  p << "-c" << "user.name=test" << "-c" << "user.email=test@test" << "-c" << "commit.gpgsign=false";
  for (auto const& a : args) p << a;
  p.start();
  auto out = pc.outs();
  while (!out.empty() && out.back() == '\n') out.pop_back();
  return out;
}

void commit(fs::path const& dir, std::string const& file, std::string const& text) {
  std::ofstream(dir / file) << text;
  git(dir, {"add", file});
  git(dir, {"commit", "-q", "-m", file + "_" + text});  // Process args are split on spaces
}

// a remote with branch latest, a work tree that pushes to it, and a clone of latest made by co()
struct Rolling {
  fs::path const root = fs::temp_directory_path() / ("mkn.kul.scm." + std::to_string(::getpid()));
  fs::path const remote = root / "remote.git", work = root / "work", clone = root / "clone";
  std::string const url = "file://" + remote.string();
  mkn::kul::scm::Git const g;

  Rolling() {
    fs::remove_all(root);
    fs::create_directories(work);
    git(root, {"init", "-q", "--bare", remote.string()});
    git(work, {"init", "-q", "-b", "latest"});
    commit(work, "a", "1");
    git(work, {"push", "-q", url, "latest"});
    g.co(clone.string(), url, "latest");
  }
  ~Rolling() { fs::remove_all(root); }

  [[nodiscard]] std::string head(fs::path const& d) const { return git(d, {"rev-parse", "HEAD"}); }
  void up() const { g.up(clone.string(), url, "latest"); }
  void rebuild() const {  // latest made again from its parent, pushed by force
    git(work, {"reset", "-q", "--hard", "HEAD~1"});
    commit(work, "b", "rebuilt");
    git(work, {"push", "-q", "-f", url, "latest"});
  }
};

}  // namespace

TEST(SCM_Git, UpFastForwards) {
  Rolling r;
  commit(r.work, "a", "2");
  git(r.work, {"push", "-q", r.url, "latest"});
  r.up();
  EXPECT_EQ(r.head(r.clone), r.head(r.work));
}

TEST(SCM_Git, UpFollowsARebuiltBranch) {
  Rolling r;
  commit(r.work, "a", "2");
  git(r.work, {"push", "-q", r.url, "latest"});
  r.up();
  r.rebuild();
  r.up();
  EXPECT_EQ(r.head(r.clone), r.head(r.work));
}

TEST(SCM_Git, UpKeepsLocalCommits) {
  Rolling r;
  commit(r.work, "a", "2");
  git(r.work, {"push", "-q", r.url, "latest"});
  r.up();
  commit(r.clone, "mine", "1");
  auto const mine = r.head(r.clone);
  r.rebuild();
  EXPECT_THROW(r.up(), mkn::kul::scm::Exception);
  EXPECT_EQ(r.head(r.clone), mine);
}

TEST(SCM_Git, UpKeepsLocalChanges) {
  Rolling r;
  commit(r.work, "a", "2");
  git(r.work, {"push", "-q", r.url, "latest"});
  r.up();
  auto const was = r.head(r.clone);
  std::ofstream(r.clone / "a") << "changed";
  r.rebuild();
  EXPECT_THROW(r.up(), mkn::kul::scm::Exception);
  EXPECT_EQ(r.head(r.clone), was);
}
#endif
