
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
  std::vector<size_t> vals{1, 5, 5, 3, 4, 5};
  for (size_t i = 0; i < 6; i++) raw[i] = vals[i];
  size_t i = 0;
  for (auto const& span : spanset)
    for (auto const& d0 : span) EXPECT_EQ(d0, vals[i++]);
}
