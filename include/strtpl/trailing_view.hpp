/// @file trailing_view.hpp
#include <iterator> // std::iterator_traits
#include <ranges>
#include <type_traits>
#include <utility>

namespace strtpl {
  template <std::ranges::input_range View>
  requires std::ranges::view<View>
  struct trailing_view : std::ranges::view_interface<trailing_view<View>> {
  private:
    [[no_unique_address]] View base_ = View();
    std::ranges::range_difference_t<View> count_ = 0;

    template <bool Const>
    class iterator;

  public:
    trailing_view() requires std::default_initializable<View>
    = default;
    constexpr trailing_view(View base, std::ranges::range_difference_t<View> count)
      : base_(std::move(base)), count_(std::move(count)) {
      if (std::ranges::empty(base_))
        count_ = 0;
      assert(count_ >= 0);
    }

    constexpr View
    base() const& noexcept requires std::copy_constructible<View> {
      return base_;
    }
    constexpr View
    base() && {
      return std::move(base_);
    }
    constexpr std::ranges::range_difference_t<View>
    count() const noexcept {
      return count_;
    }

    constexpr iterator<false>
    begin() {
      return {*this};
    }
    constexpr iterator<true>
    begin() const requires std::ranges::input_range<const View> {
      return {*this};
    }

    constexpr auto
    end() const {
      return std::default_sentinel;
    }

    constexpr auto
    size() requires std::ranges::sized_range<View> {
      return std::ranges::empty(base_)
               ? 0
               : std::ranges::size(base_)
                   + static_cast<std::ranges::range_size_t<View>>(count_ - 1);
    }
    constexpr auto
    size() const requires std::ranges::sized_range<const View> {
      return std::ranges::empty(base_)
               ? 0
               : std::ranges::size(base_)
                   + static_cast<std::ranges::range_size_t<const View>>(count_ - 1);
    }
  };

  template <class Range, class DifferenceType>
  trailing_view(Range&&, DifferenceType) -> trailing_view<std::views::all_t<Range>>;

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

  template <std::ranges::input_range View>
  requires std::ranges::view<View>
  template <bool Const>
  struct trailing_view<View>::iterator
    : trailing_iterator_category<std::conditional_t<Const, const View, View>> {
  private:
    using Parent = std::conditional_t<Const, const trailing_view, trailing_view>;
    using Base = std::conditional_t<Const, const View, View>;
    template <class T>
    using FirstType = std::conditional_t<Const and std::is_lvalue_reference_v<T>,
                                         std::add_const_t<std::remove_reference_t<T>>&, T>;

    [[no_unique_address]] Parent* parent_ = nullptr;
    [[no_unique_address]] std::ranges::iterator_t<Base> current_ = std::ranges::iterator_t<Base>();
    [[no_unique_address]] std::ranges::iterator_t<Base> next_ = std::ranges::iterator_t<Base>();
    std::ranges::range_difference_t<Base> ncount_ = 0;

    constexpr bool
    accessible() const noexcept {
      return next_ != std::ranges::end(parent_->base()) or ncount_ < parent_->count();
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
    using value_type = std::pair<FirstType<std::ranges::range_reference_t<Base>>,
                                 std::ranges::range_difference_t<Base>>;

    iterator() requires std::default_initializable<std::ranges::iterator_t<Base>>
    = default;
    // clang-format off
    constexpr iterator(Parent& parent)
      : parent_(std::addressof(parent)),
        current_(std::ranges::begin(parent.base())),
        next_([&parent] {
          if (std::ranges::empty(parent.base()))
            return std::ranges::begin(parent.base());
          return std::ranges::next(std::ranges::begin(parent.base()));
        }()) {}
    // clang-format on

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

    constexpr std::pair<FirstType<std::ranges::range_reference_t<Base>>,
                        std::ranges::range_difference_t<Base>>
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
        current_ = next_++;
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
        next_ = current_--;
      } else {
        --ncount_;
      }
      return *this;
    }
    constexpr iterator
    operator--(int) requires std::ranges::forward_range<Base> {
      auto tmp = *this;
      --*this;
      return tmp;
    }

    friend constexpr bool
    operator==(const iterator& x,
               const iterator& y) requires std::equality_comparable<std::ranges::iterator_t<Base>> {
      return x.current_ == y.current_ and x.ncount_ == y.ncount_;
    }
    friend constexpr bool
    operator==(const iterator& x, std::default_sentinel_t) requires
      std::equality_comparable<std::ranges::iterator_t<Base>> {
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
