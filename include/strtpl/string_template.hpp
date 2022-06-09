/// @file string_template.hpp
#include <algorithm>  // std::copy
#include <exception>  // std::out_of_range, std::runtime_error
#include <functional> // std::invoke
#include <iterator>   // std::iterator_traits, std::back_inserter, std::distance
#include <regex>
#include <string>
#include <string_view>
#include <type_traits> // std::remove_cvref_t
#include <utility>

namespace strtpl {

  // match_results_format

  template <class BidirectionalIter, class Allocator, class OutputIter, class ST>
  OutputIter
  match_results_format(
    const std::match_results<BidirectionalIter, Allocator>& mo, OutputIter out,
    std::basic_string_view<typename std::iterator_traits<BidirectionalIter>::value_type, ST> fmt,
    std::regex_constants::match_flag_type flags = std::regex_constants::format_default) {
    return mo.format(out, fmt.data(), fmt.data() + fmt.size(), flags);
  }

  // regex_escape

  template <class CharT, class ST>
  std::basic_string<CharT, ST>
  regex_escape(std::basic_string_view<CharT, ST> s,
               std::regex_constants::match_flag_type flags = std::regex_constants::match_default) {
    std::basic_string<CharT, ST> r;
    // See https://developer.mozilla.org/en-US/docs/Web/JavaScript/Guide/Regular_Expressions
    const std::basic_regex<CharT> re{R"([.*+?^${}()|[\]\\])"};
    std::regex_replace(std::back_inserter(r), s.begin(), s.end(), re, "\\$&", flags);
    return r;
  }

  // regex_replace_fn

  template <class T, class CharT>
  struct is_std_basic_string_view_with_char_type : std::false_type {};

  template <class ST, class CharT>
  struct is_std_basic_string_view_with_char_type<std::basic_string_view<CharT, ST>, CharT>
    : std::true_type {};

  template <class BidirectionalIter, class Traits, class CharT, class Fn>
  inline constexpr bool regex_replace_fn_constraint = std::conjunction_v<
    std::is_invocable<Fn&, const std::match_results<BidirectionalIter>&>,
    is_std_basic_string_view_with_char_type<
      std::invoke_result_t<Fn&, const std::match_results<BidirectionalIter>&>, CharT>>;

  // clang-format off
  template <class OutputIter, class BidirectionalIter, class Traits, class CharT, class Fn>
  requires regex_replace_fn_constraint<BidirectionalIter, Traits, CharT, Fn>
  OutputIter
  // clang-format on
  regex_replace_fn(
    OutputIter out, BidirectionalIter first, BidirectionalIter last,
    const std::basic_regex<CharT, Traits>& re, Fn fn,
    std::regex_constants::match_flag_type flags = std::regex_constants::match_default) {
    using Iter = std::regex_iterator<BidirectionalIter, CharT, Traits>;
    Iter i(first, last, re, flags);
    Iter eof;
    const bool format_copy = !(flags & std::regex_constants::format_no_copy);
    if (i == eof) {
      if (format_copy)
        out = std::copy(first, last, out);
    } else {
      std::sub_match<BidirectionalIter> lm;
      const bool format_first_only = flags & std::regex_constants::format_first_only;
      for (; i != eof; ++i) {
        if (format_copy)
          out = std::copy(i->prefix().first, i->prefix().second, out);
        out = match_results_format(*i, out, std::invoke(fn, *i), flags);
        lm = i->suffix();
        if (format_first_only)
          break;
      }
      if (format_copy)
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

  // regex_count

  template <class BidirectionalIter, class Traits, class CharT>
  std::pair<std::ptrdiff_t, std::ptrdiff_t>
  regex_count(BidirectionalIter first, BidirectionalIter last,
              const std::basic_regex<CharT, Traits>& re,
              std::regex_constants::match_flag_type flags = std::regex_constants::match_default) {
    using Iter = std::regex_iterator<BidirectionalIter, CharT, Traits>;
    Iter i(first, last, re, flags);
    Iter eof;
    std::ptrdiff_t n = 0, m = 0;
    if (i == eof) {
      m = std::distance(first, last);
    } else {
      std::sub_match<BidirectionalIter> lm;
      const bool format_first_only = flags & std::regex_constants::format_first_only;
      for (; i != eof; ++i) {
        ++n;
        lm = i->suffix();
        if (format_first_only)
          break;
      }
      m = lm.length();
    }
    return {n, m};
  }

  template <class Traits, class CharT, class ST>
  std::pair<std::ptrdiff_t, std::ptrdiff_t>
  regex_count(std::basic_string_view<CharT, ST> s, const std::basic_regex<CharT, Traits>& re,
              std::regex_constants::match_flag_type flags = std::regex_constants::match_default) {
    return regex_count(s.begin(), s.end(), re, flags);
  }

  // substitute

  namespace hidden_ops::inline string_view_ops {
    template <class CharT, class Traits, class Allocator>
    std::basic_string<CharT, Traits, Allocator>
    operator+(std::basic_string<CharT, Traits, Allocator>&& lhs,
              std::basic_string_view<CharT, Traits> rhs) {
      return std::move(lhs.append(rhs));
    }
  } // namespace hidden_ops::inline string_view_ops

  template <class Map, class Key>
  concept map_with_key_type = requires(Map& map, const Key& key) {
    get<1>(*map.find(key));
  };

  template <class Map, class Key>
  using map_mapped_t = decltype(get<1>(*std::declval<Map&>().find(std::declval<const Key&>())));

  // clang-format off
  template <class Map, class Key>
  requires map_with_key_type<Map, Key>
  constexpr decltype(auto)
  // clang-format on
  at(Map& map, const Key& key) {
    auto i = map.find(key);
    using std::end;
    return i == end(map) ? throw std::out_of_range("strtpl::at") : get<1>(*i);
  }

  template <class CharT>
  struct string_template {
    using char_type = CharT;

    std::basic_string_view<CharT> delimiter{};
    std::basic_string_view<CharT> idpattern{};
    std::basic_string_view<CharT> braceidpattern{};
    const std::basic_string_view<CharT> invalid{"()"};
    std::regex_constants::match_flag_type flags = std::regex_constants::match_default;

    string_template() = default;
    // clang-format off
    constexpr string_template(std::basic_string_view<CharT> delim,
                              std::basic_string_view<CharT> id)
      : delimiter{delim}, idpattern{id}, braceidpattern{id} {}
    constexpr string_template(std::basic_string_view<CharT> delim,
                              std::basic_string_view<CharT> id,
                              std::basic_string_view<CharT> bid,
                              std::regex_constants::match_flag_type f = std::regex_constants::match_default)
      : delimiter{delim}, idpattern{id}, braceidpattern{bid}, flags{f} {}
    // clang-format on

  private:
    template <class BidirectionalIter>
    static void
    _invalid(const std::match_results<BidirectionalIter>& mo) {
      // See https://docs.python.org/ja/3/library/stdtypes.html#str.splitlines
      const std::basic_regex<CharT> re{R"((\r\n?|[\n\v\f]))"};
      const auto [lineno, colno] = regex_count(mo.prefix().first, mo.prefix().second, re);
      const auto msg = "Invalid placeholder in string: line " + std::to_string(lineno + 1)
                       + ", col " + std::to_string(colno + 1);
      throw std::runtime_error(msg);
    }

  public:
    // clang-format off
    template <class ST, class Map>
    requires map_with_key_type<Map, std::basic_string_view<CharT, ST>>
    std::basic_string<CharT, ST>
    // clang-format on
    operator()(std::basic_string_view<CharT, ST> s, const Map& map) const {
      const std::basic_regex<CharT> re{[this] {
        using namespace hidden_ops::string_view_ops;
        const auto delim = regex_escape(delimiter);
        const auto escape = "(" + delim + ")";
        return delim + "(?:" + idpattern + "|\\{" + braceidpattern + "\\}|" + escape + "|" + invalid
               + ")";
      }()};
      const auto convert =
        [this,
         &map](const std::match_results<typename std::basic_string_view<CharT, ST>::iterator>& mo)
        -> std::remove_cvref_t<map_mapped_t<Map, std::basic_string_view<CharT, ST>>> {
        if (mo[1].matched) {
          std::basic_string_view<CharT, ST> key(std::to_address(mo[1].first),
                                                static_cast<std::size_t>(mo[1].length()));
          return at(map, key);
        } else if (mo[2].matched) {
          std::basic_string_view<CharT, ST> key(std::to_address(mo[2].first),
                                                static_cast<std::size_t>(mo[2].length()));
          return at(map, key);
        } else if (mo[3].matched) {
          return delimiter;
        } else if (mo[4].matched) {
          _invalid(mo);
        }
        throw std::runtime_error("Unrecognized group in pattern");
      };
      return regex_replace_fn(s, re, convert, flags);
    }
  }; // struct string_template

  inline namespace cpo {
    // See https://github.com/python/cpython/blob/971343eb569a3418aa9a0bad9b638cccf1470ef8/Lib/string.py#L57
    inline constexpr string_template<char> substitute{"$", "([_a-zA-Z][_a-zA-Z0-9]*)"};
  } // namespace cpo
} // namespace strtpl
