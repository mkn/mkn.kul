

TEST(QSort, ascending) {
  std::vector<std::size_t> v{1, 2, 3, 1, 2, 3, 3, 2, 1, 6, 1, 6, 4,
                             3, 7, 8, 9, 2, 3, 4, 5, 6, 7, 8, 9};
  mkn::kul::sort(v);
  auto prev = v[0];
  for (std::size_t i = 1; i < v.size(); ++i) {
    EXPECT_LE(prev, v[i]);
    prev = v[i];
  }
}

TEST(QSort, half) {
  std::vector<std::size_t> v{1, 2, 3, 1, 2, 3, 3, 2, 1, 6, 1, 6, 4,
                             3, 7, 8, 9, 2, 3, 4, 5, 6, 7, 8, 9};
  auto start = v.size() / 2;
  auto end = v.size() - 1;
  mkn::kul::sort(v, start, end);
  auto prev = v[start];
  for (std::size_t i = start + 1; i < v.size(); ++i) {
    EXPECT_LE(prev, v[i]);
    prev = v[i];
  }
}

TEST(QSort, descending) {
  auto constexpr ascending = false;
  std::vector<std::size_t> v{1, 2, 3, 1, 2, 3, 3, 2, 1, 6, 1, 6, 4,
                             3, 7, 8, 9, 2, 3, 4, 5, 6, 7, 8, 9};
  mkn::kul::sort<ascending>(v);
  auto prev = v[0];
  for (std::size_t i = 1; i < v.size(); ++i) {
    EXPECT_GE(prev, v[i]);
    prev = v[i];
  }
}

TEST(QSort, parallel_ascending0) {  // 128 values
  std::vector<std::size_t> v{1, 2, 3, 1, 2, 3, 3, 2, 1, 6, 1, 6, 4, 1, 2, 3, 1, 2, 3, 3, 2, 1,
                             6, 1, 6, 4, 1, 2, 3, 1, 2, 3, 3, 2, 1, 6, 1, 6, 4, 1, 2, 3, 1, 2,
                             3, 3, 2, 1, 6, 1, 6, 4, 1, 2, 3, 1, 2, 3, 3, 2, 1, 6, 1, 6, 4, 1,
                             2, 3, 1, 2, 3, 3, 2, 1, 6, 1, 6, 4, 3, 7, 8, 9, 2, 3, 4, 5, 6, 7,
                             8, 9, 1, 2, 3, 1, 2, 3, 3, 2, 1, 6, 1, 6, 4, 1, 2, 3, 2, 3, 3, 2,
                             1, 6, 1, 6, 4, 1, 2, 3, 1, 2, 3, 3, 2, 1, 6, 1, 6, 4};
  mkn::kul::para_sort(v);
  auto prev = v[0];
  for (std::size_t i = 1; i < v.size(); ++i) {
    EXPECT_LE(prev, v[i]);
    prev = v[i];
  }
}

TEST(QSort, parallel_ascending1) {  // 126 values
  std::vector<std::size_t> v{1, 2, 3, 1, 2, 3, 3, 2, 1, 6, 1, 6, 4, 1, 2, 3, 1, 2, 3, 3, 2,
                             1, 6, 1, 6, 4, 1, 2, 3, 1, 2, 3, 3, 2, 1, 6, 1, 6, 4, 1, 2, 3,
                             1, 2, 3, 3, 2, 1, 6, 1, 6, 4, 1, 2, 3, 1, 2, 3, 3, 2, 1, 6, 1,
                             6, 4, 1, 2, 3, 1, 2, 3, 3, 2, 1, 6, 1, 6, 4, 3, 7, 8, 9, 2, 3,
                             4, 5, 6, 7, 8, 9, 1, 2, 3, 1, 2, 3, 3, 2, 1, 6, 1, 6, 4, 1, 2,
                             3, 2, 3, 3, 2, 1, 6, 1, 6, 4, 1, 2, 3, 1, 2, 3, 3, 2, 1, 6, 1};
  mkn::kul::para_sort(v);
  auto prev = v[0];
  for (std::size_t i = 1; i < v.size(); ++i) {
    EXPECT_LE(prev, v[i]);
    prev = v[i];
  }
}

TEST(QSort, parallel_ascending2) {  // 4 values
                                    // defaults to serial if elements are smaller than a cache line
  std::vector<std::size_t> v{1, 2, 3, 1};
  mkn::kul::para_sort(v);
  auto prev = v[0];
  for (std::size_t i = 1; i < v.size(); ++i) {
    EXPECT_LE(prev, v[i]);
    prev = v[i];
  }
}

TEST(QSort, parallel_ascending3) {  // 128 values
  std::vector<std::size_t> v{1, 2, 3, 1, 2, 3, 3, 2, 1, 6, 1, 6, 4, 1, 2, 3, 1, 2, 3, 3, 2, 1,
                             6, 1, 6, 4, 1, 2, 3, 1, 2, 3, 3, 2, 1, 6, 1, 6, 4, 1, 2, 3, 1, 2,
                             3, 3, 2, 1, 6, 1, 6, 4, 1, 2, 3, 1, 2, 3, 3, 2, 1, 6, 1, 6, 4, 1,
                             2, 3, 1, 2, 3, 3, 2, 1, 6, 1, 6, 4, 3, 7, 8, 9, 2, 3, 4, 5, 6, 7,
                             8, 9, 1, 2, 3, 1, 2, 3, 3, 2, 1, 6, 1, 6, 4, 1, 2, 3, 2, 3, 3, 2,
                             1, 6, 1, 6, 4, 1, 2, 3, 1, 2, 3, 3, 2, 1, 6, 1, 6, 4};
  mkn::kul::para_sort(v, 3);
  auto prev = v[0];
  for (std::size_t i = 1; i < v.size(); ++i) {
    EXPECT_LE(prev, v[i]);
    prev = v[i];
  }
}

TEST(QSort, parallel_ascending4) {  // 126 values
  std::vector<std::size_t> v{1, 2, 3, 1, 2, 3, 3, 2, 1, 6, 1, 6, 4, 1, 2, 3, 1, 2, 3, 3, 2,
                             1, 6, 1, 6, 4, 1, 2, 3, 1, 2, 3, 3, 2, 1, 6, 1, 6, 4, 1, 2, 3,
                             1, 2, 3, 3, 2, 1, 6, 1, 6, 4, 1, 2, 3, 1, 2, 3, 3, 2, 1, 6, 1,
                             6, 4, 1, 2, 3, 1, 2, 3, 3, 2, 1, 6, 1, 6, 4, 3, 7, 8, 9, 2, 3,
                             4, 5, 6, 7, 8, 9, 1, 2, 3, 1, 2, 3, 3, 2, 1, 6, 1, 6, 4, 1, 2,
                             3, 2, 3, 3, 2, 1, 6, 1, 6, 4, 1, 2, 3, 1, 2, 3, 3, 2, 1, 6, 1};
  mkn::kul::para_sort(v, 3);
  auto prev = v[0];
  for (std::size_t i = 1; i < v.size(); ++i) {
    EXPECT_LE(prev, v[i]);
    prev = v[i];
  }
}

auto constexpr arr_LT_fn = [](auto const& x, auto const& y) {
  for (std::uint16_t i = 0; i < x.size(); ++i)
    if (x[i] < y[i])
      return true;
    else if (x[i] != y[i])
      return false;
  return false;
};

auto constexpr arr_GT_fn = [](auto const& x, auto const& y) {
  for (std::uint16_t i = 0; i < x.size(); ++i)
    if (x[i] > y[i])
      return true;
    else if (x[i] != y[i])
      return false;
  return false;
};

TEST(QSort, ascending_array) {
  using C = std::vector<std::array<std::size_t, 3>>;
  // clang-format off
  C v{{1, 2, 3}, {3, 3, 3}, {2, 1, 2}, {3, 1, 3},
       {1, 3, 1}, {0, 2, 0}, {2, 0, 2}, {0, 0, 1},
       {1, 1, 0}, {4, 0, 4}, {0, 0, 4}, {4,4,0}
  };
  // clang-format on

  mkn::kul::sort(v, arr_LT_fn, arr_GT_fn);

  auto prev = v[0];
  for (std::size_t i = 1; i < v.size(); ++i) {
    EXPECT_LE(prev, v[i]);
    prev = v[i];
  }
}

TEST(QSort, ascending_array1) {
  using C = std::vector<std::array<std::size_t, 3>>;
  // clang-format off
  C v{{1, 2, 3}, {3, 3, 3}, {2, 1, 2}, {3, 1, 3},
       {1, 3, 1}, {0, 2, 0}, {2, 0, 2}, {0, 0, 1},
       {1, 1, 0}, {4, 0, 4}, {0, 0, 4}, {4,4,0}
  };
  // clang-format on

  mkn::kul::sort(v);  // will break in C++20?

  auto prev = v[0];
  for (std::size_t i = 1; i < v.size(); ++i) {
    EXPECT_LE(prev, v[i]);
    prev = v[i];
  }
}

TEST(QSort, para_ascending_array1) {
  using C = std::vector<std::array<std::size_t, 3>>;
  // clang-format off
  C v{{1, 2, 3}, {3, 3, 3}, {2, 1, 2}, {3, 1, 3},
       {1, 3, 1}, {0, 2, 0}, {2, 0, 2}, {0, 0, 1},
       {1, 1, 0}, {4, 0, 4}, {0, 0, 4}, {4,4,0},
       {1, 2, 3}, {3, 3, 3}, {2, 1, 2}, {3, 1, 3},
       {1, 3, 1}, {0, 2, 0}, {2, 0, 2}, {0, 0, 1},
       {1, 1, 0}, {4, 0, 4}, {0, 0, 4}, {4,4,0},
       {1, 2, 3}, {3, 3, 3}, {2, 1, 2}, {3, 1, 3},
       {1, 3, 1}, {0, 2, 0}, {2, 0, 2}, {0, 0, 1}
  };
  // clang-format on

  mkn::kul::para_sort(v);  // will break in C++20?

  auto prev = v[0];
  for (std::size_t i = 1; i < v.size(); ++i) {
    EXPECT_LE(prev, v[i]);
    prev = v[i];
  }
}
