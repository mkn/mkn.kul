

TEST(StringOperations, SplitByChar) {
  std::vector<std::string> v;
  kul::String::SPLIT("split - by - char - dash", '-', v);
  EXPECT_EQ((size_t)4, v.size());
  EXPECT_EQ("split ", v[0]);
  EXPECT_EQ(" by ", v[1]);
  EXPECT_EQ(" char ", v[2]);
  EXPECT_EQ(" dash", v[3]);
}

TEST(StringOperations, SplitByString) {
  std::vector<std::string> v;
  kul::String::SPLIT("split - by - char - dash", "-", v);
  EXPECT_EQ((size_t)4, v.size());
  EXPECT_EQ("split ", v[0]);
  EXPECT_EQ(" by ", v[1]);
  EXPECT_EQ(" char ", v[2]);
  EXPECT_EQ(" dash", v[3]);
}

TEST(StringOperations, SplitByEscapedChar) {
  std::vector<std::string> v;
  kul::String::ESC_SPLIT("split \\- by - char - dash", '-', v);
  EXPECT_EQ((size_t)3, v.size());
  EXPECT_EQ("split - by ", v[0]);
  EXPECT_EQ(" char ", v[1]);
  EXPECT_EQ(" dash", v[2]);
}

TEST(StringOperations, String_2_UInt16_t_invalid_tooLarge) {
  tryCatch(
      {// toolarge
       []() { kul::String::UINT16(std::to_string((std::numeric_limits<uint16_t>::max)() + 10)); }},
      true);
}
TEST(StringOperations, String_2_UInt16_t_invalid_tooNegative) {
  tryCatch(
      {// tooNegative
       []() { kul::String::UINT16("-1"); }},
      true);
}
TEST(StringOperations, String_2_UInt16_t_invalid_hasLetters) {
  tryCatch({// contains letters * 3
            []() { kul::String::UINT16("a2c"); }, []() { kul::String::UINT16("1bc"); },
            []() { kul::String::UINT16("ab3"); }},
           true);
}
TEST(StringOperations, String_2_UInt16_t_valid) {
  tryCatch(
      {// valid
       []() { kul::String::UINT16("100"); }},
      false);
}

TEST(StringOperations, String_2_Int16_t_invalid_tooLarge) {
  tryCatch(
      {// toolarge
       []() { kul::String::INT16(std::to_string((std::numeric_limits<int16_t>::max)() + 10)); }},
      true);
}
TEST(StringOperations, String_2_Int16_t_invalid_tooNegative) {
  tryCatch(
      {// tooNegative
       []() {
         kul::String::INT16(std::to_string(((std::numeric_limits<int16_t>::max)() + 10) * -1));
       }},
      true);
}
TEST(StringOperations, String_2_Int16_t_invalid_hasLetters) {
  tryCatch({// contains letters * 3
            []() { kul::String::INT16("a2c"); }, []() { kul::String::INT16("1bc"); },
            []() { kul::String::INT16("ab3"); }},
           true);
}
TEST(StringOperations, String_2_Int16_t_valid) {
  tryCatch({// valid
            []() { kul::String::INT16("100"); }, []() { kul::String::INT16("-100"); }},
           false);
}

TEST(StringOperations, String_2_UInt32_t_invalid_tooLarge) {
  tryCatch(
      {// toolarge
       []() {
         uint64_t val = (std::numeric_limits<uint32_t>::max)();
         val += 10;
         kul::String::UINT32(std::to_string(val));
       }},
      true);
}
TEST(StringOperations, String_2_UInt32_t_invalid_tooNegative) {
  tryCatch(
      {// tooNegative
       []() { kul::String::UINT32(std::to_string(-1)); }},
      true);
}
TEST(StringOperations, String_2_UInt32_t_invalid_hasLetters) {
  tryCatch({// contains letters * 3
            []() { kul::String::UINT32("a2c"); }, []() { kul::String::UINT32("1bc"); },
            []() { kul::String::UINT32("ab3"); }},
           true);
}
TEST(StringOperations, String_2_UInt32_t_valid) {
  tryCatch(
      {// valid
       []() { kul::String::UINT32("100"); }},
      false);
}

TEST(StringOperations, String_2_Int32_t_invalid_tooLarge) {
  tryCatch(
      {// toolarge
       []() {
         int64_t val = (std::numeric_limits<int32_t>::max)();
         val += 10;
         kul::String::INT32(std::to_string(val));
       }},
      true);
}
TEST(StringOperations, String_2_Int32_t_invalid_tooNegative) {
  tryCatch(
      {// tooNegative
       []() {
         int64_t val = (std::numeric_limits<int32_t>::max)();
         val += 10;
         kul::String::INT32("-" + std::to_string(val));
       }},
      true);
}
TEST(StringOperations, String_2_Int32_t_invalid_hasLetters) {
  tryCatch({// contains letters * 3
            []() { kul::String::INT32("a2c"); }, []() { kul::String::INT32("1bc"); },
            []() { kul::String::INT32("ab3"); }},
           true);
}
TEST(StringOperations, String_2_Int32_t_valid) {
  tryCatch({// valid
            []() { kul::String::INT32("100"); }, []() { kul::String::INT32("-100"); }},
           false);
}

TEST(StringOperations, String_2_UInt64_t_invalid_tooLarge) {
  tryCatch(
      {// toolarge
       []() {
         uint64_t val = (std::numeric_limits<uint64_t>::max)();
         kul::String::UINT64(std::to_string(val) + "0");
       }},
      true);
}
TEST(StringOperations, String_2_UInt64_t_invalid_tooNegative) {
  tryCatch(
      {// tooNegative
       []() {
         uint64_t val = (std::numeric_limits<uint64_t>::max)();
         kul::String::UINT64("-" + std::to_string(val) + "0");
       }},
      true);
}
TEST(StringOperations, String_2_UInt64_t_invalid_hasLetters) {
  tryCatch({// contains letters * 3
            []() { kul::String::UINT64("a2c"); }, []() { kul::String::UINT64("1bc"); },
            []() { kul::String::UINT64("ab3"); }},
           true);
}
TEST(StringOperations, String_2_UInt64_t_valid) {
  tryCatch(
      {// valid
       []() { kul::String::UINT64(std::to_string((std::numeric_limits<uint64_t>::max)())); }},
      false);
}

TEST(StringOperations, String_2_Int64_t_invalid) {
  tryCatch({// toolarge
            []() {
              uint64_t val = (std::numeric_limits<uint64_t>::max)();
              kul::String::INT64(std::to_string(val) + "0");
            },
            // tooNegative
            []() { kul::String::INT64(std::to_string(-1)); },
            // contains letters * 3
            []() { kul::String::INT64("a2c"); }, []() { kul::String::INT64("1bc"); },
            []() { kul::String::INT64("ab3"); }},
           true);
}
TEST(StringOperations, String_2_Int64_t_valid) {
  tryCatch(
      {// valid
       []() { kul::String::INT64(std::to_string((std::numeric_limits<int64_t>::max)())); }},
      false);
}
