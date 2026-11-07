

#include "test_common.hpp"

#include "mkn/kul/span.hpp"

TEST(Span, init) {
  {
    std::vector<double> v{1, 2, 3};
    mkn::kul::Span<double> span0{v};
    double i = 1;
    for (auto const& d0 : span0) EXPECT_EQ(d0, i++);

    mkn::kul::Span<double> span1{span0};
    i = 1;
    for (auto const& d0 : span1) EXPECT_EQ(d0, i++);
  }
  std::vector<double> const v{1, 2, 3};
  mkn::kul::Span<double const> span{v};
  double i = 1;
  for (auto const& d0 : span) EXPECT_EQ(d0, i++);
}

TEST(SpanSet, containsCorrectData) {
  mkn::kul::SpanSet<double> spanset{std::vector<size_t>{2, 2, 2}};
  mkn::kul::Span<double> raw = spanset.raw();
  std::vector<double> vals{1, 5, 5, 3, 4, 5};
  for (size_t i = 0; i < 6; i++) raw[i] = vals[i];
  size_t i = 0;
  for (auto const& span : spanset)
    for (auto const& d0 : span) EXPECT_EQ(d0, vals[i++]);
}

TEST(SpanSet, unequalSizes) {
  mkn::kul::SpanSet<double> spanset{std::vector<size_t>{1, 2, 3}};
  EXPECT_EQ(spanset.size(), 6u);
  EXPECT_EQ(spanset.displs(), (std::vector<size_t>{0, 1, 3}));

  auto raw = spanset.raw();
  for (size_t i = 0; i < raw.size(); i++) raw[i] = static_cast<double>(i);  // 0 | 1 2 | 3 4 5

  std::vector<std::vector<double>> const expected{{0}, {1, 2}, {3, 4, 5}};
  for (size_t i = 0; i < expected.size(); i++) {
    auto const span = spanset[i];
    EXPECT_EQ(std::vector<double>(span.begin(), span.end()), expected[i]) << i;
  }

  size_t i = 0;
  for (auto const& span : spanset)
    EXPECT_EQ(std::vector<double>(span.begin(), span.end()), expected[i++]);
  EXPECT_EQ(i, expected.size());

  auto const& cspanset = spanset;
  i = 0;
  for (auto const& span : cspanset)
    EXPECT_EQ(std::vector<double>(span.begin(), span.end()), expected[i++]);
  EXPECT_EQ(i, expected.size());
}

TEST(SpanSet, iterators) {
  mkn::kul::SpanSet<double> spanset{std::vector<size_t>{2, 0, 1}};
  auto it = spanset.begin();
  EXPECT_TRUE(it != spanset.end());
  EXPECT_EQ((*it).size(), 2u);
  EXPECT_EQ((*++it).size(), 0u);  // empty span in the middle
  EXPECT_EQ((*++it).size(), 1u);
  EXPECT_TRUE(++it == spanset.end());
  EXPECT_TRUE(spanset.cbegin() != spanset.cend());

  mkn::kul::SpanSet<double> const empty{std::vector<size_t>{}};
  EXPECT_TRUE(empty.begin() == empty.end());
}
