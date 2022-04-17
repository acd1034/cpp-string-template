#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <ranges>
#include <string_view>
#include <unordered_map>
#include <strtpl/regex.hpp>

TEST_CASE("regex", "[regex]") {
  namespace regex = strtpl::regex;
  { // regex_range
    std::string_view s = "abc123def123ghi";
    const std::regex re{R"(\d+)"};
    std::ranges::common_range auto r = regex::regex_range(s, re);
    for (const auto& mr : r) {
      CHECK(mr.format("$0") == "123");
    }
  } // namespace strtpl::regex;
  { // regex_replace_fn
    std::string_view s = "abc123def456ghi789";
    const std::regex re{R"(\d+)"};
    constexpr auto fn = [](auto&&) -> std::string_view { return "exa"; };
    CHECK(regex::regex_replace_fn(s, re, fn) == "abcexadefexaghiexa");
  }
}

TEST_CASE("regex v2", "[regex][v2]") {
  namespace regex = strtpl::regex::v2;
  { // regex_split
    std::string_view s = "abc123def456ghi789";
    const std::regex re{R"(\d+)"};
    std::string r;
    std::ranges::copy(regex::regex_split(s, re) | std::views::join, std::back_inserter(r));
    CHECK(r == "abcdefghi");
  } // namespace strtpl::regex::v2;
  {
    std::string_view s = "23+68*45-96/12";
    const std::regex re{R"([\+\-\*/])"};
    std::string r;
    std::ranges::copy(regex::regex_split(s, re) | std::views::join, std::back_inserter(r));
    CHECK(r == "2368459612");
  }
  {
    std::string_view s = "23+68*45-96/12";
    const std::regex re{R"([\+\-\*/])"};
    std::string r;
    std::ranges::copy(regex::regex_split(s, re, true) | std::views::join, std::back_inserter(r));
    CHECK(r == s);
  }
  {
    std::string_view s = "23+68*45-96/12";
    const std::regex re{R"([\+\-\*/])"};
    std::string r;
    std::ranges::copy(regex::regex_split_n(s, re, 1) | std::views::join, std::back_inserter(r));
    CHECK(r == "2368*45-96/12");
  }
}
