#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <ranges>
#include <string_view>
#include <unordered_map>
#include <strtpl/regex.hpp>
namespace regex = strtpl::regex;

TEST_CASE("regex", "[regex]") {
  { // regex_range
    std::string_view s = "abc123def123ghi";
    const std::regex re{R"(\d+)"};
    std::ranges::common_range auto r = regex::regex_range(s, re);
    for (const auto& mr : r) {
      CHECK(mr.format("$0") == "123");
    }
  }
  { // regex_replace_fn
    std::string_view s = "abc123def";
    const std::regex re{R"(\d+)"};
    constexpr auto fn = [](auto&&) -> std::string_view { return "example"; };
    CHECK(regex::regex_replace_fn(s, re, fn) == "abcexampledef");
  }
}
