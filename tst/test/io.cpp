

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
  char c[21];
  c[20] = 'X';  // read must not write past the 20 bytes asked for
  mkn::kul::File file("LICENSE.md");
  if (!file) KEXCEPT(mkn::kul::Exception, "ReadFile: FileNotFound: ", file.full());
  mkn::kul::io::Reader r("LICENSE.md");
  auto n = r.read(c, 20);
  ASSERT_EQ(n, 20u);
  EXPECT_EQ("Copyright (c) 2026, ", std::string(c, n));
  EXPECT_EQ(c[20], 'X');
  n = r.read(c, 20);
  ASSERT_EQ(n, 20u);
  EXPECT_EQ("Philip Deegan.\nAll r", std::string(c, n));
  EXPECT_EQ(c[20], 'X');
}

TEST(IO_Test, ReadFileEnd) {
  mkn::kul::io::Reader r("LICENSE.md");
  std::string all;
  char c[64];
  for (std::size_t n; (n = r.read(c, sizeof(c)));) all.append(c, n);
  EXPECT_EQ(all.size(), mkn::kul::File("LICENSE.md").size());
  EXPECT_EQ(r.read(c, sizeof(c)), 0u);
}

// Travis has a tough time with this one
#if defined(MKN_TEST_BINARY_READING)
TEST(IO_Test, ReadBinaryFileLine) {
  mkn::kul::io::BinaryReader r("LICENSE.md");
  char const* c = r.readLine();
  std::string s1 = c, s2;
  while ((c = r.readLine())) s2 = c;
  EXPECT_EQ("Copyright (c) 2026, Philip Deegan.", s1);
  EXPECT_EQ("OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.", s2);
}
TEST(IO_Test, ReadBinaryFile) {
  char c[20];
  mkn::kul::io::BinaryReader r("LICENSE.md");
  auto n = r.read(c, 20);
  EXPECT_EQ("Copyright (c) 2026, ", std::string(c, n));
  n = r.read(c, 20);
  EXPECT_EQ("Philip Deegan.\nAll r", std::string(c, n));
}
#endif
