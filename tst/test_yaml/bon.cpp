
#include "gtest/gtest.h"

#include "mkn/kul/bon/yaml.hpp"
#include "mkn/kul/yaml.hpp"

#include <filesystem>
#include <fstream>
#include <string>

// bon::from: bon to YAML::Node, the parser itself is tested in tst/test/bon.cpp

TEST(BonYaml, map) {
  auto const n = mkn::kul::bon::from("{a: b, c: {d: e}}");
  ASSERT_TRUE(n.IsMap());
  EXPECT_EQ(n["a"].as<std::string>(), "b");
  ASSERT_TRUE(n["c"].IsMap());
  EXPECT_EQ(n["c"]["d"].as<std::string>(), "e");
}

TEST(BonYaml, list) {
  auto const n = mkn::kul::bon::from("{a: {x, y, z}}");
  ASSERT_TRUE(n["a"].IsSequence());
  ASSERT_EQ(n["a"].size(), 3u);
  EXPECT_EQ(n["a"][2].as<std::string>(), "z");
}

TEST(BonYaml, scalarAndEmpty) {
  auto const n = mkn::kul::bon::from("{rofl: {copter}, e: {}}");
  ASSERT_TRUE(n["rofl"].IsScalar());
  EXPECT_EQ(n["rofl"].as<std::string>(), "copter");
  ASSERT_TRUE(n["e"].IsMap());
  EXPECT_EQ(n["e"].size(), 0u);
}

TEST(BonYaml, modArgs) {  // as maiken uses it: name{phase{k: v}}
  YAML::Node node;
  for (auto const p : mkn::kul::bon::from("{compile{fix: --nocomments, system_last: true}}"))
    node[p.first] = p.second;
  EXPECT_EQ(node["compile"]["fix"].as<std::string>(), "--nocomments");
  EXPECT_EQ(node["compile"]["system_last"].as<std::string>(), "true");
}

TEST(BonYaml, fromYaml) {  // a module's YAML config as maiken hands it on
  auto const y = YAML::Load("compile: {paths: inc, n: 2}\nlist: [a, b]\nnone:\n");
  auto const b = mkn::kul::bon::from_yaml(y);
  ASSERT_TRUE(b.is_map());
  EXPECT_EQ(b["compile"]["paths"].str(), "inc");
  EXPECT_EQ(b["compile"]["n"].str(), "2");
  ASSERT_TRUE(b["list"].is_list());
  EXPECT_EQ(b["list"][1].str(), "b");
  EXPECT_TRUE(b["none"].is_map());  // null: an empty map
  EXPECT_EQ(b["none"].size(), 0u);
  EXPECT_EQ(b.find("missing"), nullptr);
  EXPECT_EQ(mkn::kul::bon::from_yaml(YAML::Node{}).size(), 0u);
  // and back
  EXPECT_EQ(mkn::kul::bon::to_yaml(b)["compile"]["paths"].as<std::string>(), "inc");
}

TEST(BonYaml, malformed) { EXPECT_THROW(mkn::kul::bon::from("{a: b"), mkn::kul::bon::Exception); }

int main(int argc, char* argv[]) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

namespace {
class NamedYaml : public mkn::kul::yaml::File {
 public:
  NamedYaml(std::filesystem::path const& p) : mkn::kul::yaml::File(p) {}
  mkn::kul::yaml::Validator validator() const override {
    return mkn::kul::yaml::Validator({mkn::kul::yaml::NodeValidator("name", 1)});
  }
};
}  // namespace

TEST(Yaml, createFromPath) {
  std::filesystem::path const dir{"bin/test/kul yaml"};  // a space
  std::filesystem::create_directories(dir);
  std::ofstream(dir / "ok.yaml") << "name: x\n";
  std::ofstream(dir / "bad.yaml") << "nom: x\n";

  auto const ok = mkn::kul::yaml::File::CREATE<NamedYaml>(dir / "ok.yaml");
  EXPECT_EQ(ok.root()["name"].as<std::string>(), "x");
  EXPECT_EQ(ok.file(), (dir / "ok.yaml").string());
  EXPECT_ANY_THROW(mkn::kul::yaml::File::CREATE<NamedYaml>(dir / "bad.yaml"));
  std::filesystem::remove_all(dir);
}
