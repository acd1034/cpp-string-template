/// @file regex.hpp
#include <algorithm>  // std::ranges::copy, std::copy
#include <functional> // std::invoke
#include <iterator>   // std::iterator_traits, std::back_inserter
#include <ranges>     // std::ranges::subrange
#include <regex>
#include <string_view>
#include <type_traits> // std::remove_cvref_t
#include <utility>
#include <strtpl/trailing_view.hpp>

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

  template <class BiIter, class Allocator, class ST>
  auto
  match_results_format(
    const std::match_results<BiIter, Allocator>& mr,
    std::basic_string_view<typename std::iterator_traits<BiIter>::value_type, ST> fmt,
    std::regex_constants::match_flag_type flags = std::regex_constants::format_default) {
    std::basic_string<typename std::iterator_traits<BiIter>::value_type, ST> r;
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

  template <class CharT, class ST>
  struct is_std_basic_string_view_with_char_type<std::basic_string_view<CharT, ST>, CharT>
    : std::true_type {};

  template <class CharT, class ST, class Fn,
            class BiIter = typename std::basic_string_view<CharT, ST>::iterator>
  inline constexpr bool regex_replace_fn_constraint =
    std::conjunction_v<std::is_invocable<Fn&, const std::match_results<BiIter>&>,
                       is_std_basic_string_view_with_char_type<
                         std::invoke_result_t<Fn&, const std::match_results<BiIter>&>, CharT>>;

  // clang-format off
  template <class OutputIter, class Traits, class CharT, class ST, class Fn>
  requires regex_replace_fn_constraint<CharT, ST, Fn>
  OutputIter
  // clang-format on
  regex_replace_fn(
    OutputIter out, std::basic_string_view<CharT, ST> s, const std::basic_regex<CharT, Traits>& re,
    Fn fn, std::regex_constants::match_flag_type flags = std::regex_constants::match_default) {
    auto r = trailing_view(regex_range(s, re, flags), 2);
    const bool format_copy = !(flags & std::regex_constants::format_no_copy);
    if (r.empty()) {
      if (format_copy)
        out = std::ranges::copy(s, out).out;
    } else {
      const bool format_first_only = flags & std::regex_constants::format_first_only;
      for (const auto& [mr, last] : r) {
        if (last) {
          out = std::copy(mr.suffix().first, mr.suffix().second, out);
          break;
        }
        if (format_copy)
          out = std::copy(mr.prefix().first, mr.prefix().second, out);
        out = match_results_format(mr, out, std::invoke(fn, mr), flags);
        if (format_first_only) {
          out = std::copy(mr.suffix().first, mr.suffix().second, out);
          break;
        }
      }
    }
    return out;
  }

  // clang-format off
  template <class Traits, class CharT, class ST, class Fn>
  requires regex_replace_fn_constraint<CharT, ST, Fn>
  std::basic_string<CharT, ST>
  // clang-format on
  regex_replace_fn(
    std::basic_string_view<CharT, ST> s, const std::basic_regex<CharT, Traits>& re, Fn fn,
    std::regex_constants::match_flag_type flags = std::regex_constants::match_default) {
    std::basic_string<CharT, ST> r;
    regex_replace_fn(std::back_inserter(r), s, re, fn, flags);
    return r;
  }
} // namespace strtpl::regex

namespace strtpl::regex::v2 {

  // another version of regex_replace_fn

  /* // clang-format off
  template <class Traits, class CharT, class ST, class Fn>
  requires regex_replace_fn_constraint<CharT, ST, Fn>
  auto
  // clang-format on
  regex_replace_fn(
    std::basic_string_view<CharT, ST> s, const std::basic_regex<CharT, Traits>& re, Fn fn,
    std::regex_constants::match_flag_type flags = std::regex_constants::match_default) {
    const auto gn = [&fn](const auto& x) {
      const auto& [mr, last] = x;
      if (last) {
        auto r = std::ranges::subrange(mr.suffix().first, mr.suffix().second);
        return std::ranges::join_view(std::vector{r}); // crrently error because GCC does not support `owning_view`
      }
      auto r1 = std::ranges::subrange(mr.prefix().first, mr.prefix().second);
      auto r2 = std::ranges::subrange(std::invoke(fn, mr));
      return std::ranges::join_view(std::vector{r1, r2}); // crrently error because GCC does not support `owning_view`
    };
    return trailing_view(regex_range(s, re, flags), 2) | std::views::transform(gn)
           | std::views::join;
  } */

  // regex_split

  template <class Traits, class CharT, class ST>
  auto
  regex_split(std::basic_string_view<CharT, ST> s, const std::basic_regex<CharT, Traits>& re,
              std::regex_constants::match_flag_type flags = std::regex_constants::match_default) {
    constexpr auto fn = [](const auto& x) {
      const auto& [mr, last] = x;
      if (last)
        return std::ranges::subrange(mr.suffix().first, mr.suffix().second);
      return std::ranges::subrange(mr.prefix().first, mr.prefix().second);
    };
    return trailing_view(regex_range(s, re, flags), 2) | std::views::transform(fn);
  }
} // namespace strtpl::regex::v2
