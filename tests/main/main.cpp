#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <string_view>
#include <unordered_map>
#include <strtpl/string_template.hpp>

TEST_CASE("main", "[main]") {
  { // regex_escape
    std::string_view s = "$C++$";
    CHECK(strtpl::regex_escape(s) == R"(\$C\+\+\$)");
  }
  { // regex_replace_fn
    std::string_view s = "abc123def";
    const std::regex re{R"(\d+)"};
    constexpr auto fn = [](auto&&) -> std::string_view { return "example"; };
    CHECK(strtpl::regex_replace_fn(s, re, fn) == "abcexampledef");
  }
  { // regex_count
    std::string_view s = "abc123"
                         "def";
    const std::regex re{R"(\d+)"};
    const auto [n, m] = strtpl::regex_count(s, re);
    CHECK(n == 1);
    CHECK(m == 3);
  }
  { // substitute
    std::unordered_map<std::string_view, std::string_view> map{
      {"what", "example"},
    };
    std::string_view s1 = "This is $what.";
    std::string_view s2 = "This is ${what}ified.";
    std::string_view s3 = "This is dollar $$.";
    std::string_view s4 = "This is error $.";
    std::string_view s5 = "This is error too $which.";
    CHECK(strtpl::substitute(s1, map) == "This is example.");
    CHECK(strtpl::substitute(s2, map) == "This is exampleified.");
    CHECK(strtpl::substitute(s3, map) == "This is dollar $.");
    CHECK_THROWS_AS(strtpl::substitute(s4, map), std::runtime_error);
    CHECK_THROWS_AS(strtpl::substitute(s5, map), std::out_of_range);
  }
}
