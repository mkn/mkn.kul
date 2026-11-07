#include "test_common.hpp"

#include "mkn/kul/bon.hpp"

#include <string>
#include <vector>

namespace {

mkn::kul::bon::Node bon(std::string const& s) { return mkn::kul::bon::parse(s); }

std::string str(mkn::kul::bon::Node const& n) { return n.str(); }

}  // namespace

TEST(Bon, map) {
  auto const n = bon("{a: b, c: d}");
  ASSERT_TRUE(n.is_map());
  EXPECT_EQ(n.size(), 2u);
  EXPECT_EQ(str(n["a"]), "b");
  EXPECT_EQ(str(n["c"]), "d");
}

TEST(Bon, whitespaceTrimmed) {
  auto const n = bon("  { a :  b c  ,\n\td:e }  ");
  EXPECT_EQ(str(n["a"]), "b c");
  EXPECT_EQ(str(n["d"]), "e");
}

TEST(Bon, nested) {
  auto const n = bon("{a: {b: c, d: {e: f}}, g: h}");
  EXPECT_EQ(str(n["a"]["b"]), "c");
  EXPECT_EQ(str(n["a"]["d"]["e"]), "f");
  EXPECT_EQ(str(n["g"]), "h");
}

TEST(Bon, siblingObjects) {
  auto const n = bon("{a: {b: c}, d: {e: f}, g: {h: i}}");
  EXPECT_EQ(str(n["a"]["b"]), "c");
  EXPECT_EQ(str(n["d"]["e"]), "f");
  EXPECT_EQ(str(n["g"]["h"]), "i");
}

TEST(Bon, keyDirectlyBeforeObject) {  // maiken module args: name{phase{k: v}}
  auto const n = bon("{compile{fix: --nocomments --nosafe_headers, system_last: true}}");
  EXPECT_EQ(str(n["compile"]["fix"]), "--nocomments --nosafe_headers");
  EXPECT_EQ(str(n["compile"]["system_last"]), "true");

  auto const m = bon("{init {style: file, paths: .}, compile{args: -DX}}");
  EXPECT_EQ(str(m["init"]["style"]), "file");
  EXPECT_EQ(str(m["init"]["paths"]), ".");
  EXPECT_EQ(str(m["compile"]["args"]), "-DX");
}

TEST(Bon, list) {
  auto const n = bon("{a: {x, y, z}}");
  ASSERT_TRUE(n["a"].is_list());
  ASSERT_EQ(n["a"].size(), 3u);
  EXPECT_EQ(str(n["a"][0]), "x");
  EXPECT_EQ(str(n["a"][1]), "y");
  EXPECT_EQ(str(n["a"][2]), "z");

  auto const top = bon("{x, y}");
  ASSERT_TRUE(top.is_list());
  EXPECT_EQ(top.size(), 2u);
}

TEST(Bon, singleEntryIsScalar) {
  auto const n = bon("{rofl: {copter}}");
  ASSERT_TRUE(n["rofl"].is_string());
  EXPECT_EQ(str(n["rofl"]), "copter");
}

TEST(Bon, valueWithColon) {
  auto const n = bon("{url: http://x.y:8080/z, arg: -DX=a:b}");
  EXPECT_EQ(str(n["url"]), "http://x.y:8080/z");
  EXPECT_EQ(str(n["arg"]), "-DX=a:b");
}

TEST(Bon, escapes) {
  auto const n = bon(R"({a: b\, c, k\:ey: \{v\}, s: x\\y, t: z\ })");
  EXPECT_EQ(str(n["a"]), "b, c");
  EXPECT_EQ(str(n["k:ey"]), "{v}");
  EXPECT_EQ(str(n["s"]), R"(x\y)");
  EXPECT_EQ(str(n["t"]), "z ");  // escaped trailing space is kept
}

TEST(Bon, empty) {
  auto const n = bon("{}");
  EXPECT_TRUE(n.is_map());
  EXPECT_EQ(n.size(), 0u);

  auto const m = bon("{a: {}}");
  EXPECT_TRUE(m["a"].is_map());
  EXPECT_EQ(m["a"].size(), 0u);

  EXPECT_EQ(str(bon("{a:}")["a"]), "");
}

TEST(Bon, trailingComma) {
  auto const n = bon("{a: b, c: {d: e,},}");
  EXPECT_EQ(str(n["a"]), "b");
  EXPECT_EQ(str(n["c"]["d"]), "e");
}

TEST(Bon, duplicateKeyLastWins) { EXPECT_EQ(str(bon("{a: 1, a: 2}")["a"]), "2"); }

TEST(Bon, malformed) {
  for (std::string const s : {
           "",                // empty
           "a: b",            // no braces
           "{a: b",           // unclosed
           "{a: {b: c}",      // unclosed nested
           "{a: b}}",         // extra '}'
           "{a: b}}}",        // extra '}'s
           "{a: b}{c: d}",    // two objects
           "{a: b}, {c: d}",  // two objects
           "{{a: b}}",        // object without key
           "{a: {{b: c}}}",   // nested object without key
           "{: b}",           // empty key
           "{a: b, , c: d}",  // empty entry
           "{a: b, c}",       // list entry in a map
           "{c, a: b}",       // map entry in a list
           "{a: b{c}}",       // '{' in a value
           "{a: {b} c}",      // text after an object
           "{a: b\\",         // escape at end of input
       })
    EXPECT_THROW(bon(s), mkn::kul::bon::Exception) << s;
}

TEST(Bon, keyOrderKept) {
  auto const n = bon("{z: 1, a: 2, m: 3}");
  std::vector<std::string> keys;
  for (auto const& [k, v] : n.map()) keys.push_back(k);
  EXPECT_EQ(keys, (std::vector<std::string>{"z", "a", "m"}));
}

TEST(Bon, nodeAccess) {
  auto const n = bon("{a: b, l: {x, y}}");
  EXPECT_TRUE(n.contains("a"));
  EXPECT_FALSE(n.contains("q"));
  EXPECT_EQ(n.size(), 2u);
  EXPECT_EQ(n["l"].size(), 2u);
  EXPECT_EQ(n["l"][1].str(), "y");
  EXPECT_EQ(n["a"].size(), 1u);
  EXPECT_FALSE(n["a"].contains("b"));  // not a map
  EXPECT_THROW(n["q"], mkn::kul::bon::Exception);
  EXPECT_THROW(n["l"][2], mkn::kul::bon::Exception);
  EXPECT_THROW(n["a"].list(), mkn::kul::bon::Exception);
  EXPECT_THROW(n["l"].str(), mkn::kul::bon::Exception);
  EXPECT_THROW(n.str(), mkn::kul::bon::Exception);
}
