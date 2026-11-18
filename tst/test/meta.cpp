#include "test_common.hpp"

#include "mkn/kul/int.hpp"
#include "mkn/kul/meta.hpp"
#include "mkn/kul/float.hpp"
#include "mkn/kul/tuple.hpp"
#include "mkn/kul/equality.hpp"
#include "mkn/kul/constexprifier.hpp"

#include <array>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>
#include <type_traits>

static_assert(mkn::kul::all_are<int>(1, 2));
static_assert(!mkn::kul::all_are<int>(1, 2.));
static_assert(mkn::kul::any_are<double>(1, 2.));
static_assert(mkn::kul::is_any_of<float, int>(1));
static_assert(!mkn::kul::is_any_of<int, float, double>());
static_assert(mkn::kul::IsStdVector<std::vector<int>>);
static_assert(!mkn::kul::IsStdVector<std::array<int, 2>>);
static_assert(mkn::kul::IsStdArray<std::array<int, 2>>);
static_assert(!mkn::kul::IsStdArray<int>);
static_assert(mkn::kul::Iterable<std::vector<int>>);
static_assert(mkn::kul::Subscriptable<int*>);
static_assert(std::is_same_v<mkn::kul::tuple_fixed_type<int, 2>, std::tuple<int, int>>);
static_assert(mkn::kul::all_in(1, 1, 1) && !mkn::kul::all_in(1, 1, 2));
static_assert(mkn::kul::any_in(2, 1, 2) && !mkn::kul::any_in(3, 1, 2));
static_assert(mkn::kul::int_equals(1, 1u) && !mkn::kul::int_equals(-1, 4294967295u));
static_assert(mkn::kul::int_equals(std::int64_t{3}, 3));

TEST(Meta, deref_aggregate) {
  int i = 1;
  int* p = &i;
  mkn::kul::deref(p) = 2;
  EXPECT_EQ(mkn::kul::deref(i), 2);

  struct Agg {
    int a, b;
  };
  auto const agg = std::make_shared<mkn::kul::aggregate_adapter<Agg>>(1, 2);
  EXPECT_EQ(agg->b, 2);
  mkn::kul::allsame(1, 2, 3);
}

TEST(Meta, tuple) {
  std::array<int, 2> const arr{1, 2};
  auto const tup = mkn::kul::as_tuple(arr);
  EXPECT_EQ(&std::get<1>(tup), &arr[1]);

  int sum = 0;
  mkn::kul::apply(std::make_tuple(1, 2, 3), [&](auto e) { sum += e; });
  EXPECT_EQ(sum, 6);

  mkn::kul::NamedTuple<int, double> named = mkn::kul::make_named_tuple(
      std::pair<std::string, int>{"i", 1}, std::pair<std::string, double>{"d", 2.});
  EXPECT_EQ(std::get<1>(named).first, "d");
}

TEST(Meta, float_equality) {
  EXPECT_TRUE(mkn::kul::float_equals(std::array{1., 2.}, std::array{1., 2.}));
  EXPECT_FALSE(mkn::kul::float_equals(std::array{1., 2.}, std::array{1., 2.1}));
  EXPECT_TRUE(mkn::kul::float_not_equals(1., 1.1));
  EXPECT_TRUE(mkn::kul::any_float_eq(1.f, 1.));

  std::vector<double> const v0{1, 2, 3}, v1{1, 2, 3.5};
  EXPECT_TRUE(
      (mkn::kul::FloatComparator<std::vector<double>, std::vector<double>>{v0, v0, 1e-15, 0, 3}()));
  mkn::kul::FloatComparator<std::vector<double>, std::vector<double>> cmp{v0, v1, 1e-15, 0, 3};
  EXPECT_FALSE(cmp());
  EXPECT_DOUBLE_EQ(cmp.max_diff, .5);

  mkn::kul::EqualityReport const report{false, "differs", 1};
  EXPECT_FALSE(report);
  EXPECT_EQ(report.why(), "differs");
}

namespace {
enum class Mode : std::uint8_t { A, B, C, count };
}

TEST(Constexprifier, lift) {
  static_assert(std::variant_size_v<mkn::kul::EnumConstantVariant_t<Mode>> == 3);
  auto constexpr table =
      mkn::kul::make_enum_table<Mode, int>({{Mode::C, 3}, {Mode::A, 1}, {Mode::B, 2}});
  static_assert(table[static_cast<std::size_t>(Mode::B)] == 2);
  EXPECT_THROW(((void)mkn::kul::make_enum_table<Mode, int>({{Mode::A, 1}})), std::runtime_error);
  EXPECT_THROW((void)mkn::kul::asEnumConstant(Mode::count), std::runtime_error);

  int seen = 0;
  mkn::kul::Constexprifier{true, Mode::C}([&]<bool b, Mode m>() {
    static_assert(std::is_same_v<decltype(b), bool>);
    if constexpr (b && m == Mode::C)
      seen = 1;
    else
      seen = 2;
  });
  EXPECT_EQ(seen, 1);
  mkn::kul::Constexprifier{false}([&]<bool b>() { seen = b ? 3 : 4; });
  EXPECT_EQ(seen, 4);
}
