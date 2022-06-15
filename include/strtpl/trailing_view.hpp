/// @file trailing_view.hpp
#include <concepts> // std::semiregular
#include <iterator> // std::iterator_traits, std::unreachable_sentinel_t
#include <ranges>
#include <type_traits>
#include <utility>

namespace strtpl {
  template <class T, class U>
  concept different_from = !std::same_as<std::remove_cvref_t<T>, std::remove_cvref_t<U>>;

  // clang-format off
  template <std::ranges::input_range View, std::semiregular Bound = std::unreachable_sentinel_t>
  requires std::ranges::view<View>
    and (std::convertible_to<Bound, std::ranges::range_difference_t<View>> or std::same_as<Bound, std::unreachable_sentinel_t>)
  struct trailing_view : std::ranges::view_interface<trailing_view<View, Bound>> {
    // clang-format on
  private:
    [[no_unique_address]] View base_ = View();
    Bound bound_ = Bound();

    template <bool Const>
    class iterator;

  public:
    trailing_view() requires std::default_initializable<View>
    = default;
    constexpr trailing_view(View base) : base_(std::move(base)) {}
    constexpr trailing_view(View base, Bound bound)
      : base_(std::move(base)), bound_(std::move(bound)) {
      assert(bound_ >= 0);
    }

    constexpr View
    base() const& noexcept requires std::copy_constructible<View> {
      return base_;
    }
    constexpr View
    base() && {
      return std::move(base_);
    }
    constexpr Bound
    count() const noexcept {
      return bound_;
    }

    constexpr iterator<false>
    begin() {
      return {*this, std::ranges::begin(base_)};
    }
    constexpr iterator<true>
    begin() const requires std::ranges::input_range<const View> {
      return {*this, std::ranges::begin(base_)};
    }

    constexpr auto
    end() {
      return std::default_sentinel;
    }
    constexpr iterator<false>
    end() requires
      std::ranges::common_range<View> and different_from<Bound, std::unreachable_sentinel_t> {
      return {*this, std::ranges::end(base_), bound_};
    }
    constexpr auto
    end() const requires std::ranges::input_range<const View> {
      return std::default_sentinel;
    }
    constexpr iterator<true>
    end() const requires std::ranges::input_range<const View> and std::ranges::common_range<
      const View> and different_from<Bound, std::unreachable_sentinel_t> {
      return {*this, std::ranges::end(base_), bound_};
    }

    constexpr auto
    size() requires
      std::ranges::sized_range<View> and different_from<Bound, std::unreachable_sentinel_t> {
      using Size = std::ranges::range_size_t<View>;
      if (std::ranges::empty(base_))
        return static_cast<Size>(0);
      return std::ranges::size(base_) + static_cast<Size>(bound_) - 1;
    }
    constexpr auto
    size() const requires
      std::ranges::sized_range<const View> and different_from<Bound, std::unreachable_sentinel_t> {
      using Size = std::ranges::range_size_t<const View>;
      if (std::ranges::empty(base_))
        return static_cast<Size>(0);
      return std::ranges::size(base_) + static_cast<Size>(bound_) - 1;
    }
  };

  template <class Range>
  trailing_view(Range&&) -> trailing_view<std::views::all_t<Range>>;

  template <class Range, class Bound>
  trailing_view(Range&&, Bound) -> trailing_view<std::views::all_t<Range>, Bound>;

  template <class View>
  struct trailing_iterator_category {};

  template <std::ranges::forward_range View>
  struct trailing_iterator_category<View> {
    using Cat = typename std::iterator_traits<std::ranges::iterator_t<View>>::iterator_category;
    // clang-format off
    using iterator_category =
      std::conditional_t<std::derived_from<Cat, std::bidirectional_iterator_tag>, std::bidirectional_iterator_tag,
      std::conditional_t<std::derived_from<Cat, std::forward_iterator_tag>,       std::forward_iterator_tag,
      /* else */                                                                  Cat>>;
    // clang-format on
  };

  // clang-format off
  template <std::ranges::input_range View, std::semiregular Bound>
  requires std::ranges::view<View>
    and (std::convertible_to<Bound, std::ranges::range_difference_t<View>> or std::same_as<Bound, std::unreachable_sentinel_t>)
  template <bool Const>
  struct trailing_view<View, Bound>::iterator
    : trailing_iterator_category<std::conditional_t<Const, const View, View>> {
    // clang-format on
  private:
    using Parent = std::conditional_t<Const, const trailing_view, trailing_view>;
    using Base = std::conditional_t<Const, const View, View>;

    [[no_unique_address]] Parent* parent_ = nullptr;
    [[no_unique_address]] std::ranges::iterator_t<Base> current_ = std::ranges::iterator_t<Base>();
    [[no_unique_address]] std::ranges::iterator_t<Base> next_ = std::ranges::iterator_t<Base>();
    std::ranges::range_difference_t<Base> ncount_ = 0;

    constexpr bool
    accessible() const noexcept {
      if (std::ranges::empty(parent_->base()))
        return false;
      return next_ != std::ranges::end(parent_->base()) or ncount_ != parent_->count();
    }

  public:
    // using iterator_category = inherited;
    // clang-format off
    using iterator_concept =
      std::conditional_t<std::ranges::bidirectional_range<Base>, std::bidirectional_iterator_tag,
      std::conditional_t<std::ranges::forward_range<Base>,       std::forward_iterator_tag,
      /* else */                                                 std::input_iterator_tag>>;
    // clang-format on
    using difference_type = std::ranges::range_difference_t<Base>;
    using value_type =
      std::pair<std::ranges::range_reference_t<Base>, std::ranges::range_difference_t<Base>>;

    iterator() requires std::default_initializable<std::ranges::iterator_t<Base>>
    = default;
    constexpr iterator(Parent& parent, std::ranges::iterator_t<Base> current,
                       std::ranges::range_difference_t<Base> ncount = 0)
      : parent_(std::addressof(parent)), current_(current),
        next_([current, b = current == std::ranges::end(parent.base())] {
          if (b)
            return current;
          return std::ranges::next(current);
        }()),
        ncount_(ncount) {}

    constexpr const std::ranges::iterator_t<Base>&
    base() const& noexcept {
      return current_;
    }
    constexpr std::ranges::iterator_t<Base>
    base() && {
      return std::move(current_);
    }
    constexpr std::ranges::range_difference_t<Base>
    count() const noexcept {
      return ncount_;
    }

    constexpr std::pair<std::ranges::range_reference_t<Base>, std::ranges::range_difference_t<Base>>
    operator*() const {
      assert(accessible());
      return {*current_, ncount_};
    }

    constexpr iterator&
    operator++() {
      assert(accessible());
      if (next_ == std::ranges::end(parent_->base())) {
        ++ncount_;
      } else {
        current_ = next_;
        ++next_;
      }
      return *this;
    }
    constexpr void
    operator++(int) {
      assert(accessible());
      ++*this;
    }
    constexpr iterator
    operator++(int) requires std::ranges::forward_range<Base> {
      assert(accessible());
      auto tmp = *this;
      ++*this;
      return tmp;
    }

    constexpr iterator&
    operator--() requires std::ranges::bidirectional_range<Base> {
      if (ncount_ == 0) {
        next_ = current_;
        --current_;
      } else {
        --ncount_;
      }
      return *this;
    }
    constexpr iterator
    operator--(int) requires std::ranges::bidirectional_range<Base> {
      auto tmp = *this;
      --*this;
      return tmp;
    }

    friend constexpr bool
    operator==(const iterator& x,
               const iterator& y) requires std::equality_comparable<std::ranges::iterator_t<Base>> {
      if (std::ranges::empty(x.parent_->base()) and std::ranges::empty(y.parent_->base()))
        return true;
      return x.next_ == y.next_ and x.ncount_ == y.ncount_;
    }
    friend constexpr bool
    operator==(const iterator& x, std::default_sentinel_t) requires
      std::equality_comparable<std::ranges::iterator_t<Base>> {
      if (std::ranges::empty(x.parent_->base()))
        return true;
      return x.next_ == std::ranges::end(x.parent_->base()) and x.ncount_ == x.parent_->count();
    }

    friend constexpr std::pair<std::ranges::range_rvalue_reference_t<Base>,
                               std::ranges::range_difference_t<Base>>
    iter_move(const iterator& x) noexcept(noexcept(std::ranges::iter_move(x.current_))) {
      assert(x.accessible());
      return {std::ranges::iter_move(x.current_), x.ncount_};
    }

    // TODO: methods for random_access_iterator
  };
} // namespace strtpl
