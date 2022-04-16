/// @file regex.hpp
#include <algorithm>  // std::copy
#include <functional> // std::invoke
#include <iterator>   // std::iterator_traits, std::back_inserter
#include <ranges>     // std::ranges::subrange
#include <regex>
#include <string>
#include <string_view>
#include <type_traits> // std::remove_cvref_t
#include <utility>

namespace strtpl::regex {

  // match_results_format

  template <class BiIter, class Allocator, class OutputIter, class ST>
  OutputIter
  match_results_format(
    const std::match_results<BiIter, Allocator>& mr, OutputIter out,
    std::basic_string_view<typename std::iterator_traits<BiIter>::value_type, ST> fmt,
    std::regex_constants::match_flag_type flags = std::regex_constants::format_default) {
    return mr.format(out, fmt.data(), fmt.data() + fmt.size(), flags);
  }

  template <class BiIter, class Allocator, class ST,
            class CharT = typename std::iterator_traits<BiIter>::value_type>
  std::basic_string<CharT, ST>
  match_results_format(
    const std::match_results<BiIter, Allocator>& mr, std::basic_string_view<CharT, ST> fmt,
    std::regex_constants::match_flag_type flags = std::regex_constants::format_default) {
    std::basic_string<CharT, ST> r;
    match_results_format(mr, std::back_inserter(r), fmt, flags);
    return r;
  }

  // regex_range

  template <class Traits, class CharT, class ST,
            class Iter = std::regex_iterator<typename std::basic_string_view<CharT, ST>::iterator,
                                             CharT, Traits>>
  std::ranges::subrange<Iter>
  regex_range(std::basic_string_view<CharT, ST> s, const std::basic_regex<CharT, Traits>& re,
              std::regex_constants::match_flag_type flags = std::regex_constants::match_default) {
    return {Iter(s.begin(), s.end(), re, flags), Iter()};
  }

  // regex_replace_fn

  template <class T, class CharT>
  struct is_std_basic_string_view_with_char_type : std::false_type {};

  template <class ST, class CharT>
  struct is_std_basic_string_view_with_char_type<std::basic_string_view<CharT, ST>, CharT>
    : std::true_type {};

  template <class BiIter, class Traits, class CharT, class Fn>
  inline constexpr bool regex_replace_fn_constraint =
    std::conjunction_v<std::is_invocable<Fn&, const std::match_results<BiIter>&>,
                       is_std_basic_string_view_with_char_type<
                         std::invoke_result_t<Fn&, const std::match_results<BiIter>&>, CharT>>;

  // clang-format off
  template <class OutputIter, class BiIter, class Traits, class CharT, class Fn>
  requires regex_replace_fn_constraint<BiIter, Traits, CharT, Fn>
  OutputIter
  // clang-format on
  regex_replace_fn(
    OutputIter out, BiIter first, BiIter last, const std::basic_regex<CharT, Traits>& re, Fn fn,
    std::regex_constants::match_flag_type flags = std::regex_constants::match_default) {
    using Iter = std::regex_iterator<BiIter, CharT, Traits>;
    Iter i(first, last, re, flags);
    Iter eof;
    if (i == eof) {
      if (!(flags & std::regex_constants::format_no_copy))
        out = std::copy(first, last, out);
    } else {
      std::sub_match<BiIter> lm;
      for (; i != eof; ++i) {
        if (!(flags & std::regex_constants::format_no_copy))
          out = std::copy(i->prefix().first, i->prefix().second, out);
        out = match_results_format(*i, out, std::invoke(fn, *i), flags);
        lm = i->suffix();
        if (flags & std::regex_constants::format_first_only)
          break;
      }
      if (!(flags & std::regex_constants::format_no_copy))
        out = std::copy(lm.first, lm.second, out);
    }
    return out;
  }

  template <class Traits, class CharT, class ST, class Fn>
  requires regex_replace_fn_constraint<typename std::basic_string_view<CharT, ST>::iterator, Traits,
                                       CharT, Fn>
    std::basic_string<CharT, ST>
    regex_replace_fn(
      std::basic_string_view<CharT, ST> s, const std::basic_regex<CharT, Traits>& re, Fn fn,
      std::regex_constants::match_flag_type flags = std::regex_constants::match_default) {
    std::basic_string<CharT, ST> r;
    regex_replace_fn(std::back_inserter(r), s.begin(), s.end(), re, fn, flags);
    return r;
  }
} // namespace strtpl::regex
