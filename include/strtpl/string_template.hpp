/// @file string_template.hpp
#include <algorithm>  // std::copy
#include <functional> // std::invoke
#include <iterator>   // std::iterator_traits, std::back_inserter
#include <regex>
#include <string>
#include <string_view>
#include <type_traits> // std::remove_cvref_t
#include <utility>

namespace strtpl {

  // regex_replace_fn

  template <class BidirectionalIter, class Allocator, class OutputIter, class ST>
  OutputIter
  regex_format(
    const std::match_results<BidirectionalIter, Allocator>& mr, OutputIter out,
    std::basic_string_view<typename std::iterator_traits<BidirectionalIter>::value_type, ST> fmt,
    std::regex_constants::match_flag_type flags = std::regex_constants::format_default) {
    return mr.format(out, fmt.data(), fmt.data() + fmt.size(), flags);
  }

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
    if (i == eof) {
      if (!(flags & std::regex_constants::format_no_copy))
        out = std::copy(first, last, out);
    } else {
      std::sub_match<BidirectionalIter> lm;
      for (; i != eof; ++i) {
        if (!(flags & std::regex_constants::format_no_copy))
          out = std::copy(i->prefix().first, i->prefix().second, out);
        out = regex_format(*i, out, std::invoke(fn, *i), flags);
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

  // substitute

  namespace hidden_ops::inline string_view_ops {
    template <class CharT, class Traits, class Allocator>
    std::basic_string<CharT, Traits, Allocator> operator+(
      std::basic_string<CharT, Traits, Allocator>&& lhs,
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
  template <class Map, class Key, class U, class T = std::remove_cvref_t<map_mapped_t<Map, Key>>>
  requires map_with_key_type<Map, Key> and std::convertible_to<U&&, T>
  T
  // clang-format on
  at_or(Map& map, const Key& key, U&& v) {
    auto i = map.find(key);
    using std::end;
    return i != end(map) ? get<1>(*i) : static_cast<T>(std::forward<U>(v));
  }

  template <class CharT>
  struct string_template {
    using char_type = CharT;

    std::basic_string_view<CharT> delimiter{};
    std::basic_string_view<CharT> idpattern{};
    std::basic_string_view<CharT> braceidpattern{};
    const std::basic_string_view<CharT> invalid{"()"};
    std::regex_constants::match_flag_type flags{std::regex_constants::match_default};

    string_template() = default;
    constexpr explicit string_template(std::basic_string_view<CharT> _delimiter,
                                       std::basic_string_view<CharT> _idpattern)
      : delimiter{_delimiter}, idpattern{_idpattern}, braceidpattern{_idpattern} {}
    constexpr explicit string_template(std::basic_string_view<CharT> _delimiter,
                                       std::basic_string_view<CharT> _idpattern,
                                       std::basic_string_view<CharT> _braceidpattern)
      : delimiter{_delimiter}, idpattern{_idpattern}, braceidpattern{_braceidpattern} {}
    constexpr explicit string_template(std::basic_string_view<CharT> _delimiter,
                                       std::basic_string_view<CharT> _idpattern,
                                       std::basic_string_view<CharT> _braceidpattern,
                                       std::regex_constants::match_flag_type _flags)
      : delimiter{_delimiter}, idpattern{_idpattern}, braceidpattern{_braceidpattern}, flags{
                                                                                         _flags} {}

    // clang-format off
    template <class ST, class Map>
    requires map_with_key_type<Map, std::basic_string_view<CharT, ST>>
    std::basic_string<CharT, ST>
    // clang-format on
    operator()(std::basic_string_view<CharT, ST> s, const Map& map) const {
      const std::basic_regex<CharT> re{[this] {
        using namespace hidden_ops::string_view_ops;
        const auto delim = std::basic_string<CharT>{"\\"} + delimiter;
        const auto escape = "(" + delim + ")";
        return delim + "(?:" + idpattern + "|\\{" + braceidpattern + "\\}|" + escape + "|" + invalid
               + ")";
      }()};
      using Iter = typename std::basic_string_view<CharT, ST>::iterator;
      const auto fn = [this, &map](const std::match_results<Iter>& mr)
        -> std::remove_cvref_t<map_mapped_t<Map, std::basic_string_view<CharT, ST>>> {
        if (mr[1].matched) {
          std::basic_string_view<CharT, ST> key(mr[1].first,
                                                static_cast<std::size_t>(mr[1].length()));
          return at_or(map, key, "NONE");
        } else if (mr[2].matched) {
          std::basic_string_view<CharT, ST> key(mr[2].first,
                                                static_cast<std::size_t>(mr[2].length()));
          return at_or(map, key, "NONE");
        } else if (mr[3].matched) {
          return delimiter;
        }
        return "ERROR";
      };
      return regex_replace_fn(s, re, fn, flags);
    }
  }; // struct string_template

  inline namespace cpo {
    inline constexpr string_template<char> substitute{"$", "([_a-zA-Z][_a-zA-Z0-9]*)"};
  }
} // namespace strtpl
