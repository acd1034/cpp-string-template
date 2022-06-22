#include <iostream>
#include <string_view>
#include <unordered_map>
#include <strtpl/string_template.hpp>

int main() {
  std::unordered_map<std::string_view, std::string_view> map{
    {"who", "Alice"},
    {"what", "bananas"},
    {"verb", "run"},
  };
  { // '$' に続く単語を発見した場合、その単語を map の key として得られる値で置き換えます。
    std::cout << strtpl::substitute("$who likes $what.", map) << std::endl;
    // → Alice likes bananas.
  }
  { // '$' と単語の後にアルファベットが続く場合は、単語を "{}" で囲みます。
    std::cout << strtpl::substitute("$who ${verb}s fast.", map) << std::endl;
    // → Alice runs fast.
  }
  // '$' に続く単語が map に含まれない場合、例外を送出します。
  try {
    std::cout << strtpl::substitute("$who likes $where.", map) << std::endl;
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    // → strtpl::at: index out of range
  }
  // '$' に続く文字が無効な場合も、例外を送出します。エラーメッセージにはエラーが生じた文字の行番号と列番号を含みます。
  try {
    std::cout << strtpl::substitute("$who likes $.", map) << std::endl;
  } catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
    // → Invalid placeholder in string: line 1, col 12
  }
}
