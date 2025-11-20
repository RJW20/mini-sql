#ifndef MINISQL_SPAN_HPP
#define MINISQL_SPAN_HPP

#include <array>
#include <cstddef>
#include <type_traits>
#include <vector>

namespace minisql {

// Lightweight implementation of C++ 20's std::span<T>.
template <typename T>
class span {
public:
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using iterator = pointer;
    using const_iterator = const value_type*;

    span() : data_{nullptr}, size_{0} {}
    span(pointer ptr, size_type size) : data_{ptr}, size_{size} {}

    template <std::size_t N>
    span(std::array<value_type, N>& arr)
        : data_{arr.data()}, size_{N} {}

    span(std::vector<value_type>& vec) 
        : data_{vec.data()}, size_{vec.size()} {}

    reference operator[](size_type idx) { return data_[idx]; }
    const_reference operator[](size_type idx) const { return data_[idx]; }

    iterator begin() noexcept { return data_; }
    iterator end() noexcept { return data_ + size_; }

    const_iterator begin() const noexcept { return data_; }
    const_iterator end() const noexcept { return data_ + size_; }

    const_iterator cbegin() const noexcept { return data_; }
    const_iterator cend() const noexcept { return data_ + size_; }

    size_type size() const noexcept { return size_; }
    bool empty() const noexcept { return !size_; }

    pointer data() noexcept { return data_; }
    const_pointer data() const noexcept { return data_; }

private:
    pointer data_;
    size_type size_;
};

} // namespace minisql

#endif // MINISQL_SPAN_HPP