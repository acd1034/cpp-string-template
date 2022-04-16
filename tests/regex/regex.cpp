#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <string_view>
#include <unordered_map>
#include <strtpl/regex.hpp>
namespace regex = strtpl::regex;

TEST_CASE("regex", "[regex]") {
  {
    std::string_view s = "abc123def";
    const std::regex re{R"(\d+)"};
    constexpr auto fn = [](auto&&) -> std::string_view { return "example"; };
    CHECK(regex::regex_replace_fn(s, re, fn) == "abcexampledef");
  }
}
