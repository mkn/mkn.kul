

TEST(Exception, String) {
  std::string full = "FAILURE";
  try {
    KEXCEPT(kul::Exception, "FAIL") << "URE";
  } catch (std::exception const& e) {
    EXPECT_EQ(e.what(), full);
  }
}
