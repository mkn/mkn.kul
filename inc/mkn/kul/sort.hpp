/**
Copyright (c) 2020, Philip Deegan.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the
distribution.
    * Neither the name of Philip Deegan nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef _MKN_KUL_SORT_HPP_
#define _MKN_KUL_SORT_HPP_

#include <thread>
#include <vector>
#include <cstdint>
#include <algorithm>

namespace mkn::kul::detail {

auto constexpr LT_fn = [](auto const& x, auto const& y) constexpr { return x < y; };
using LT = decltype(LT_fn);

auto constexpr GT_fn = [](auto const& x, auto const& y) constexpr { return x > y; };
using GT = decltype(GT_fn);

auto constexpr accessor_fn = [](auto& c) constexpr -> auto& { return c; };
using Accessor = decltype(accessor_fn);

auto constexpr swaper_fn = [](auto& x, auto& y) constexpr { std::swap(x, y); };
using Swaper = decltype(swaper_fn);

}  // namespace mkn::kul::detail

namespace mkn::kul {

template <typename C, bool ascending, typename LT, typename GT,
          typename Accessor = detail::Accessor,
          typename Swaper = detail::Swaper,  //
          typename S = std::int64_t>
class QuickSorter {
 public:
  QuickSorter(C& c_, LT const& lt_, GT const& gt_,
              Accessor accessor_ = detail::accessor_fn,  //
              Swaper swaper_ = detail::swaper_fn)
      : c{c_}, lt{lt_}, gt{gt_}, accessor{accessor_}, swaper{swaper_} {}

  void sort(S l, S r) {
    auto i = l, j = r;
    auto const half = v[(l + r) / 2];
    do {
      while (fn0(v[i], half)) i++;
      while (fn1(v[j], half)) j--;
      if (i <= j) {
        swaper(v_[i], v_[j]);
        i++, j--;
      }
    } while (i <= j);
    if (l < j) sort(l, j);
    if (i < r) sort(i, r);
  }

  void sort() { sort(0, c.size() - 1); }

 private:
  C& c;

  LT const& lt;
  GT const& gt;

  Accessor const& accessor;
  Swaper const& swaper;

  decltype(accessor(c))& v_ = accessor(c);
  decltype(accessor(c)) const& v = accessor(c);

  auto static constexpr _fn0(LT const& lt_, GT const& gt_) -> auto const& {
    if constexpr (ascending)
      return lt_;
    else
      return gt_;
  }
  auto static constexpr _fn1(LT const& lt_, GT const& gt_) -> auto const& {
    if constexpr (ascending)
      return gt_;
    else
      return lt_;
  }

  using Fn0 = std::conditional_t<ascending, LT, GT>;
  using Fn1 = std::conditional_t<ascending, GT, LT>;
  Fn0 const& fn0 = _fn0(lt, gt);
  Fn1 const& fn1 = _fn1(lt, gt);
};

template <bool ascending = true, typename Container, typename S = std::int64_t>
void sort(Container& c, S start, S end) {
  using QS = QuickSorter<Container, ascending, detail::LT, detail::GT>;
  QS{c, detail::LT_fn, detail::GT_fn}.sort(start, end);
}

template <bool ascending = true, typename Container, typename LT, typename GT>
void sort(Container& c, LT const& lt, GT const& gt) {
  using QS = QuickSorter<Container, ascending, LT, GT>;
  QS{c, lt, gt}.sort();
}

template <bool ascending = true, typename Container>
void sort(Container& c) {
  sort<ascending>(c, detail::LT_fn, detail::GT_fn);
}

template <bool ascending = true, std::uint8_t CL = 64, typename Container, typename LT, typename GT>
void para_sort(Container& c, LT const& lt, GT const& gt, std::uint8_t n_threads = 2) {
  using V = typename Container::value_type;
  auto constexpr PT = CL / sizeof(V);
  if (PT < 1) n_threads = 1;  // no point

  auto const bytes = c.size() * sizeof(V);
  auto const even = bytes % CL == 0;
  auto const cls = bytes / CL + (even ? 0 : 1);
  auto const off = (cls / n_threads) * PT;

  std::vector<std::thread> threads;
  threads.reserve(n_threads - 1);
  std::vector<std::size_t> starts(n_threads, 0);
  std::vector<std::size_t> ends(n_threads, c.size() - 1);

  for (std::size_t i = 1; i < n_threads; ++i) {
    starts[i] = off * i;
    ends[i - 1] = starts[i] - 1;
  }

  using QS = QuickSorter<Container, ascending, LT, GT>;
  for (std::size_t i = 1; i < n_threads; ++i)
    threads.emplace_back([&, i]() { QS{c, lt, gt}.sort(starts[i], ends[i]); });

  QS{c, lt, gt}.sort(starts[0], ends[0]);
  for (auto& thread : threads)
    if (thread.joinable()) thread.join();

  if (n_threads > 1) QS{c, lt, gt}.sort();
}

template <bool ascending = true, std::uint8_t CL = 64, typename Container>
void para_sort(Container& c, std::uint8_t n_threads = 2) {
  para_sort<ascending, CL>(c, detail::LT_fn, detail::GT_fn, n_threads);
}

}  // namespace mkn::kul

#endif /* _MKN_KUL_SORT_HPP_ */
