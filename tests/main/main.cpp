#define CATCH_CONFIG_MAIN
#include <algorithm>
#include <catch2/catch.hpp>

TEST_CASE("test name", "[label1][label2]") {}

TEMPLATE_TEST_CASE("algorithm equal", "[algorithm][equal]", (std::array<int, 3>),
                   std::forward_list<int>, std::list<int>, std::vector<int>, int[]) {
  TestType a{1, 2, 3};
  CHECK(std::ranges::equal(a, a));
}
