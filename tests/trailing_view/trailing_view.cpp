#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <algorithm> // std::ranges::equal
#include <vector>
#include <strtpl/trailing_view.hpp>

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
    CHECK(std::ranges::equal(tv | std::views::transform(access), v));
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
    CHECK(std::ranges::equal(tv | std::views::transform(fn),
                             std::views::iota(0, std::ranges::ssize(v) + 1)));
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
  if (n == 0) {
    CHECK(std::ranges::empty(tv));
  } else {
    CHECK(std::ranges::distance(tv) == n + m - 1);
    auto it = std::ranges::begin(tv);
    for (std::ptrdiff_t i = 0; i < n - 1; ++i) {
      const auto& [x, count] = *it;
      CHECK(x == v[i]);
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
