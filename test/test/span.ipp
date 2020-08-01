

TEST(SpanSet, containsCorrectData) {
  kul::SpanSet<double> spanset{std::vector<size_t>{2, 2, 2}};
  kul::Span<double> raw = spanset.raw();
  std::vector<size_t> vals{1, 5, 5, 3, 4, 5};
  for (size_t i = 0; i < 6; i++) raw[i] = vals[i];
  size_t i = 0;
  for (auto const& span : spanset)
    for (auto const& d0 : span) EXPECT_EQ(d0, vals[i++]);
}
