#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>

#include <string_view>
#include <unordered_map>
#include <tpl/string_template.hpp>

TEST_CASE("main", "[main]") {
  {
    std::string_view s = "abc123def";
    const std::regex re{R"(\d+)"};
    constexpr auto fn = [](auto&&) -> std::string_view { return "example"; };
    CHECK(tpl::regex_replace_fn(s, re, fn) == "abcexampledef");
  }
  {
    std::unordered_map<std::string_view, std::string_view> map{
      {"what", "example"},
    };
    std::string_view s1 = "This is $what.";
    std::string_view s2 = "This is ${what}ified.";
    std::string_view s3 = "This is dollar →$$←.";
    std::string_view s4 = "This is error →$←.";
    std::string_view s5 = "This is none →$which←.";
    CHECK(tpl::substitute(s1, map) == "This is example.");
    CHECK(tpl::substitute(s2, map) == "This is exampleified.");
    CHECK(tpl::substitute(s3, map) == "This is dollar →$←.");
    CHECK(tpl::substitute(s4, map) == "This is error →ERROR←.");
    CHECK(tpl::substitute(s5, map) == "This is none →NONE←.");
  }
}
