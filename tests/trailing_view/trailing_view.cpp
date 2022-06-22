#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <algorithm> // std::ranges::equal
#include <array>
#include <vector>
#include <strtpl/trailing_view.hpp>

namespace test {

  // index

  struct index_fn {
    template <std::ranges::random_access_range RARng, class Integral>
    requires std::ranges::borrowed_range<RARng> and std::convertible_to<
      Integral, std::ranges::range_difference_t<RARng>>
    constexpr std::ranges::range_reference_t<RARng>
    operator()(RARng&& r, Integral n) const {
      return std::ranges::begin(r)[static_cast<std::ranges::range_difference_t<RARng>>(n)];
    }
  };

  inline namespace cpo {
    inline constexpr index_fn index;
  }
} // namespace test

TEST_CASE("trailing_view", "[trailing_view]") {
  constexpr auto access = [](auto&& x) -> decltype(auto) {
    return get<0>(std::forward<decltype(x)>(x));
  };
  { // static_assert
    std::vector<int> v;
    auto tv = strtpl::trailing_view{v, 0};
    using View = std::remove_cv_t<decltype(tv)>;
    using It = std::ranges::iterator_t<View>;
    static_assert(std::input_or_output_iterator<It>);
    static_assert(std::indirectly_readable<It>);
    static_assert(std::ranges::input_range<View>);
    static_assert(std::ranges::forward_range<View>);
    static_assert(std::ranges::bidirectional_range<View>);
    // static_assert(std::ranges::random_access_range<View>); // currently not supported
    static_assert(std::ranges::sized_range<View>);
    static_assert(std::ranges::common_range<View>);
    static_assert(std::assignable_from<decltype(access(*std::ranges::begin(tv))), int>);
  }
  {
    std::ranges::iota_view v{0};
    auto tv = strtpl::trailing_view{v, 0};
    using View = std::remove_cv_t<decltype(tv)>;
    using It = std::ranges::iterator_t<View>;
    static_assert(std::input_or_output_iterator<It>);
    static_assert(std::indirectly_readable<It>);
    static_assert(std::ranges::input_range<View>);
    static_assert(std::ranges::forward_range<View>);
    static_assert(std::ranges::bidirectional_range<View>);
    // static_assert(std::ranges::random_access_range<View>); // currently not supported
    static_assert(not std::ranges::sized_range<View>);
    static_assert(not std::ranges::common_range<View>);
    static_assert(not std::assignable_from<decltype(access(*std::ranges::begin(tv))), int>);
  }
  {
    const std::vector<int> v;
    auto tv = strtpl::trailing_view{v, 0};
    using View = std::remove_cv_t<decltype(tv)>;
    using It = std::ranges::iterator_t<View>;
    static_assert(std::input_or_output_iterator<It>);
    static_assert(std::indirectly_readable<It>);
    static_assert(std::ranges::input_range<View>);
    static_assert(std::ranges::forward_range<View>);
    static_assert(std::ranges::bidirectional_range<View>);
    // static_assert(std::ranges::random_access_range<View>); // currently not supported
    static_assert(std::ranges::sized_range<View>);
    static_assert(std::ranges::common_range<View>);
    static_assert(not std::assignable_from<decltype(access(*std::ranges::begin(tv))), int>);
  }
  {
    const std::ranges::iota_view v{0};
    auto tv = strtpl::trailing_view{v, 0};
    using View = std::remove_cv_t<decltype(tv)>;
    using It = std::ranges::iterator_t<View>;
    static_assert(std::input_or_output_iterator<It>);
    static_assert(std::indirectly_readable<It>);
    static_assert(std::ranges::input_range<View>);
    static_assert(std::ranges::forward_range<View>);
    static_assert(std::ranges::bidirectional_range<View>);
    // static_assert(std::ranges::random_access_range<View>); // currently not supported
    static_assert(not std::ranges::sized_range<View>);
    static_assert(not std::ranges::common_range<View>);
    static_assert(not std::assignable_from<decltype(access(*std::ranges::begin(tv))), int>);
  }
  { // check empty
    std::vector<int> v;
    auto tv = strtpl::trailing_view{v, 0};
    CHECK(std::ranges::empty(tv));
  }
  {
    std::vector<int> v;
    auto tv = strtpl::trailing_view{v, 1};
    CHECK(std::ranges::empty(tv));
  }
  {
    std::vector<int> v{0};
    auto tv = strtpl::trailing_view{v, 0};
    CHECK(std::ranges::empty(tv));
  }
  {
    std::vector<int> v{0};
    auto tv = strtpl::trailing_view{v, 1};
    CHECK_FALSE(std::ranges::empty(tv));
  }
  { // check distance
    std::vector<int> v{0, 1, 2, 3};
    auto tv = strtpl::trailing_view{v, 0};
    CHECK(std::ranges::distance(tv) == std::ranges::ssize(v) - 1);
  }
  {
    std::vector<int> v{0, 1, 2, 3};
    auto tv = strtpl::trailing_view{v, 1};
    CHECK(std::ranges::distance(tv) == std::ranges::ssize(v));
  }
  {
    std::vector<int> v{0, 1, 2, 3};
    auto tv = strtpl::trailing_view{v, 2};
    CHECK(std::ranges::distance(tv) == std::ranges::ssize(v) + 1);
  }
  { // check availability with other methods
    std::vector<int> v{0, 1, 2, 3};
    auto tv = strtpl::trailing_view{v, 1};
    CHECK(std::ranges::equal(tv, v, std::ranges::equal_to{}, access));
    CHECK(std::ranges::equal(tv | std::views::keys, v));
  }
  { // check assignment
    std::vector<int> v{0, 1, 2, 3};
    auto tv = strtpl::trailing_view{v, 2};
    for (auto&& [x, last] : tv)
      if (last)
        x = 5;
      else
        x = 4;
    std::vector<int> r{4, 4, 4, 5, 5};
    CHECK(std::ranges::equal(tv | std::views::keys, r));
  }
  { // possible use
    std::vector<std::pair<int, int>> v{
      {0, 1},
      {1, 2},
      {2, 3},
    };
    constexpr auto fn = [](const auto& x) -> int {
      const auto& [p, last] = x;
      return last ? p.second : p.first;
    };
    auto tv = strtpl::trailing_view{v, 2};
    std::vector<int> r{0, 1, 2, 3};
    CHECK(std::ranges::equal(tv | std::views::transform(fn), r));
  }
  { // possible use
    constexpr std::ptrdiff_t N = 4;
    std::vector<std::array<int, N>> v{
      {0, 1, 2, 3},
      {1, 2, 3, 4},
      {2, 3, 4, 5},
    };
    constexpr auto fn = [](const auto& x) -> int {
      const auto& [a, n] = x;
      return test::index(a, n);
    };
    auto tv = strtpl::trailing_view{v, N};
    std::vector<int> r{0, 1, 2, 3, 4, 5};
    CHECK(std::ranges::equal(tv | std::views::transform(fn), r));
  }
}

TEST_CASE("trailing_view unreachable", "[trailing_view][unreachable]") {
  {
    std::vector<int> v{0, 1, 2, 3};
    auto tv = strtpl::trailing_view(v);
    auto it = std::ranges::begin(tv);
    for (const auto& y : v) {
      const auto& [x, last] = *it++;
      CHECK(x == y);
      CHECK(last == 0);
    }
    for (std::ptrdiff_t i = 1; i < 10; ++i) {
      const auto& [x, last] = *it++;
      CHECK(x == v.back());
      CHECK(last == i);
    }
    CHECK(it != std::ranges::end(tv));
  }
}

TEST_CASE("trailing_view generators", "[trailing_view][generators]") {
  // clang-format off
  const auto v = GENERATE(
    std::vector<int>{},
    std::vector<int>{0},
    std::vector<int>{0, 1},
    std::vector<int>{0, 1, 2}
  );
  // clang-format on
  const auto m = GENERATE(0, 1, 2, 3);
  const auto n = std::ranges::ssize(v);
  auto tv = strtpl::trailing_view{v, m};
  CHECK(tv.count() == m);
  if (n == 0) {
    CHECK(std::ranges::empty(tv));
  } else {
    CHECK(std::ranges::distance(tv) == n + m - 1);
    auto it = std::ranges::begin(tv);
    for (std::ptrdiff_t i = 0; i < n - 1; ++i) {
      const auto& [x, count] = *it;
      CHECK(x == test::index(v, i));
      CHECK(count == 0);
      ++it;
    }
    for (std::ptrdiff_t i = 0; i < m; ++i) {
      const auto& [x, count] = *it;
      CHECK(x == v.back());
      CHECK(count == i);
      ++it;
    }
    CHECK(it == std::ranges::end(tv));
  }
}
