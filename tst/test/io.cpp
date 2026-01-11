

#include "test_common.hpp"

#include "mkn/kul/io.hpp"

TEST(IO_Test, ReadFileLine) {
  mkn::kul::io::Reader r("LICENSE.md");
  char const* c = r.readLine();
  std::string s1 = c, s2;
  while ((c = r.readLine())) s2 = c;
  EXPECT_EQ("Copyright (c) 2026, Philip Deegan.", s1);
  EXPECT_EQ("OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.", s2);
}
TEST(IO_Test, ReadFile) {
  char c[20] = {0};
  bzero(c);
  mkn::kul::File file("LICENSE.md");
  if (!file) KEXCEPT(mkn::kul::Exception, "ReadFile: FileNotFound: ") << file.full();
  mkn::kul::io::Reader r("LICENSE.md");
  r.read(c, 20);
  std::string s1 = c;
  std::stringstream ss;
  while ((r.read(c, 20))) {
    ss << c;
    break;
  }
  EXPECT_EQ("Copyright (c) 2026, ", s1);
  EXPECT_EQ("Philip Deegan.\nAll r", ss.str());
}

// Travis has a tough time with this one
#if defined(_MKN_TEST_BINARY_READING_)
TEST(IO_Test, ReadBinaryFileLine) {
  mkn::kul::io::BinaryReader r("LICENSE.md");
  char const* c = r.readLine();
  std::string s1 = c, s2;
  while ((c = r.readLine())) s2 = c;
  EXPECT_EQ("Copyright (c) 2026, Philip Deegan.", s1);
  EXPECT_EQ("OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.", s2);
}
TEST(IO_Test, ReadBinaryFile) {
  char c[20] = {0};
  bzero(c);
  mkn::kul::io::BinaryReader r("LICENSE.md");
  r.read(c, 20);
  std::string s1 = c;
  KLOG(INF) << s1.size();
  KLOG(INF) << strlen(c);
  std::stringstream ss;
  while ((r.read(c, 20))) {
    ss << c;
    break;
  }
  EXPECT_EQ("Copyright (c) 2026, ", s1);
  EXPECT_EQ("Philip Deegan.\nAll r", ss.str());
}
#endif
