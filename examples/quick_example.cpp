#include <iostream>
#include <string_view>
#include <unordered_map>
#include <strtpl/string_template.hpp>

template <class Map>
void substitute_test(std::string_view str, const Map& map) {
  try {
    std::cout << strtpl::substitute(str, map) << std::endl;
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
  }
}

int main() {
  std::unordered_map<std::string_view, std::string_view> map{
    {"who", "Alice"},
    {"what", "banana"},
    {"verb", "run"},
  };

  // '$' に続く単語を発見した場合、その単語を map の key として得られる値で置き換えます。
  substitute_test("$who likes $what.", map);
  // output: Alice likes banana.

  // '$' と単語の後にアルファベットが続く場合は、単語を "{}" で囲みます。
  substitute_test("$who ${verb}s fast.", map);
  // output: Alice runs fast.

  // '$' に続く単語が map に含まれない場合、例外を送出します。
  substitute_test("$who likes $where.", map);
  // output: Error: key not found

  // '$' に続く文字が無効な場合も、例外を送出します。
  // エラーメッセージにはエラーが生じた文字の行番号と列番号が含まれます。
  substitute_test("$who likes $.", map);
  // output: Invalid placeholder in string: line 1, col 12
}
