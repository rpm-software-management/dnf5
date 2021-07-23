/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2.1 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef LIBDNF_UTILS_SPAN_HPP
#define LIBDNF_UTILS_SPAN_HPP

#include <array>
#include <cstdint>
#include <iterator>
#include <type_traits>


namespace libdnf {

inline constexpr std::size_t DYNAMIC_EXTENT = static_cast<std::size_t>(-1);

// Used when size of span is known during compile time. Size is not stored in the object.
template <typename T, std::size_t Extent = DYNAMIC_EXTENT>
class Span {
public:
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T *;
    using const_pointer = const T *;
    using reference = T &;
    using const_reference = const T &;
    using iterator = T *;
    using reverse_iterator = std::reverse_iterator<iterator>;

    /// Constructs an empty span whose data() == nullptr and size() == 0.
    constexpr Span() noexcept = default;

    /// Constructs a span that is a view over the range [first, first + count);
    /// the resulting span has data() == std::to_address(first) and size() == count.
    template <class Ptr>
    constexpr explicit Span(Ptr first) noexcept : ptr{first} {}

    /// Constructs a span that is a view over the array arr;
    /// the resulting span has size() == N and data() == std::data(arr).
    constexpr explicit Span(element_type (&arr)[Extent]) noexcept { ptr = arr; }

    /// Constructs a span that is a view over the array arr;
    /// the resulting span has size() == N and data() == std::data(arr).
    constexpr explicit Span(std::array<T, Extent> & arr) noexcept { ptr = arr.data(); }

    /// Constructs a span that is a view over the array arr;
    /// the resulting span has size() == N and data() == std::data(arr).
    template <class U>
    constexpr explicit Span(const std::array<U, Extent> & arr) noexcept {
        ptr = arr.data();
    }

    /// Defaulted copy constructor copies the size and data pointer;
    /// the resulting span has size() == other.size() and data() == other.data().
    constexpr Span(const Span & other) noexcept = default;

    /// Defaulted move constructor copies the size and data pointer;
    /// the resulting span has size() == other.size() and data() == other.data().
    Span(Span && other) noexcept = default;

    ~Span() = default;

    /// Assigns other to *this. This defaulted assignment operator performs a shallow copy of the data pointer,
    /// i.e., after a call to this function, data() == other.data().
    constexpr Span & operator=(const Span & other) noexcept = default;

    /// Move other to *this. This defaulted assignment operator performs a shallow copy of the data pointer and
    /// the size, i.e., after a call to this function, data() == other.data() and size() == other.size().
    Span & operator=(Span && other) noexcept = default;

    /// Returns a reference to the first element in the span.
    /// Calling front on an empty span results in undefined behavior.
    constexpr reference front() const { return *ptr; }

    /// Returns a reference to the last element in the span.
    /// Calling back on an empty span results in undefined behavior.
    constexpr reference back() const { return *(ptr + Extent - 1); }

    /// Returns a reference to the idx-th element of the sequence.
    /// The behavior is undefined if idx is out of range (i.e., if it is greater than or equal to size()).
    constexpr reference operator[](size_type i) const { return ptr[i]; }

    /// Returns a pointer to the beginning of the sequence.
    constexpr pointer data() const noexcept { return ptr; }

    /// Returns the number of elements in the span.
    constexpr size_type size() const noexcept { return Extent; }

    /// Returns the size of the sequence in bytes.
    constexpr size_type size_bytes() const noexcept { return Extent * sizeof(element_type); }

    /// Checks if the sequence is empty.
    constexpr bool empty() const noexcept { return Extent == 0; }

    /// Returns an iterator to the first element of the span.
    /// If the span is empty, the returned iterator will be equal to end().
    constexpr iterator begin() const noexcept { return ptr; }

    /// Returns an iterator to the element following the last element of the span.
    /// This element acts as a placeholder; attempting to access it results in undefined behavior.
    constexpr iterator end() const noexcept { return ptr + Extent; }

    /// Returns a reverse iterator to the first element of the reversed span.
    /// It corresponds to the last element of the non-reversed span.
    /// If the span is empty, the returned iterator is equal to rend().
    constexpr reverse_iterator rbegin() const noexcept { return reverse_iterator(end()); }

    /// Returns a reverse iterator to the element following the last element of the reversed span.
    /// It corresponds to the element preceding the first element of the non-reversed span.
    /// This element acts as a placeholder, attempting to access it results in undefined behavior.
    constexpr reverse_iterator rend() const noexcept { return reverse_iterator(begin()); }

    /// Obtains a span that is a view over the first Count elements of this span.
    template <std::size_t Count>
    constexpr Span<element_type, Count> first() const {
        return Span<element_type, Count>(ptr);
    }

    /// Obtains a span that is a view over the first Count elements of this span.
    constexpr Span<element_type> first(size_type count) const { return Span<element_type>(ptr, count); }

    /// Obtains a span that is a view over the last Count elements of this span.
    template <std::size_t Count>
    constexpr Span<element_type, Count> last() const {
        return Span<element_type, Count>(ptr + (Extent - Count));
    }

    /// Obtains a span that is a view over the last count elements of this span.
    constexpr Span<element_type> last(size_type count) const {
        return Span<element_type>(ptr + (Extent - count), count);
    }

    /// Obtains a span that is a view over the Count elements of this span starting at offset Offset.
    /// If Count is DYNAMIC_EXTENT, the number of elements in the subspan is size() - offset.
    template <size_type Offset, size_type Count = DYNAMIC_EXTENT>
    constexpr Span<element_type, Count == DYNAMIC_EXTENT ? Extent - Offset : Count> subspan() const {
        return Span < element_type, Count == DYNAMIC_EXTENT ? Extent - Offset : Count > (ptr + Offset);
    }

    /// Obtains a span that is a view over the Count elements of this span starting at offset Offset.
    /// If Count is DYNAMIC_EXTENT, the number of elements in the subspan is size() - offset.
    constexpr Span<element_type> subspan(size_type offset, size_type count = DYNAMIC_EXTENT) const {
        return Span<element_type>(ptr + offset, count == DYNAMIC_EXTENT ? Extent - offset : count);
    }

private:
    pointer ptr{nullptr};
};

// Used when size of span is generated during runtime. Size is stored in the object.
template <typename T>
class Span<T, DYNAMIC_EXTENT> {
public:
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T *;
    using const_pointer = const T *;
    using reference = T &;
    using const_reference = const T &;
    using iterator = T *;
    using reverse_iterator = std::reverse_iterator<iterator>;

    /// Constructs an empty span whose data() == nullptr and size() == 0.
    constexpr Span() noexcept = default;

    /// Constructs a span that is a view over the range [first, first + count);
    /// the resulting span has data() == std::to_address(first) and size() == count.
    template <class Ptr>
    constexpr Span(Ptr first, size_type count) noexcept : ptr{first}
                                                        , len{count} {}

    /// Constructs a span that is a view over the array arr;
    /// the resulting span has size() == N and data() == std::data(arr).
    template <std::size_t N>
    constexpr explicit Span(element_type (&arr)[N]) noexcept {
        ptr = arr;
        len = N;
    }

    /// Constructs a span that is a view over the array arr;
    /// the resulting span has size() == N and data() == std::data(arr).
    template <std::size_t N>
    constexpr explicit Span(std::array<T, N> & arr) noexcept {
        ptr = arr.data();
        len = N;
    }

    /// Constructs a span that is a view over the array arr;
    /// the resulting span has size() == N and data() == std::data(arr).
    template <class U, std::size_t N>
    constexpr explicit Span(const std::array<U, N> & arr) noexcept {
        ptr = arr.data();
        len = N;
    }

    /// Defaulted copy constructor copies the size and data pointer;
    /// the resulting span has size() == other.size() and data() == other.data().
    constexpr Span(const Span & other) noexcept = default;

    /// Defaulted move constructor copies the size and data pointer;
    /// the resulting span has size() == other.size() and data() == other.data().
    Span(Span && other) noexcept = default;

    ~Span() = default;

    /// Assigns other to *this. This defaulted assignment operator performs a shallow copy of the data pointer and
    /// the size, i.e., after a call to this function, data() == other.data() and size() == other.size().
    constexpr Span & operator=(const Span & other) noexcept = default;

    /// Move other to *this. This defaulted assignment operator performs a shallow copy of the data pointer and
    /// the size, i.e., after a call to this function, data() == other.data() and size() == other.size().
    Span & operator=(Span && other) noexcept = default;

    /// Returns a reference to the first element in the span.
    /// Calling front on an empty span results in undefined behavior.
    constexpr reference front() const { return *ptr; }

    /// Returns a reference to the last element in the span.
    /// Calling back on an empty span results in undefined behavior.
    constexpr reference back() const { return *(ptr + len - 1); }

    /// Returns a reference to the idx-th element of the sequence.
    /// The behavior is undefined if idx is out of range (i.e., if it is greater than or equal to size()).
    constexpr reference operator[](size_type i) const { return ptr[i]; }

    /// Returns the number of elements in the span.
    constexpr pointer data() const noexcept { return ptr; }

    /// Returns the size of the sequence in bytes.
    constexpr size_type size() const noexcept { return len; }

    /// Returns the size of the sequence in bytes.
    constexpr size_type size_bytes() const noexcept { return len * sizeof(element_type); }

    /// Checks if the sequence is empty.
    constexpr bool empty() const noexcept { return len == 0; }

    /// Returns an iterator to the first element of the span.
    /// If the span is empty, the returned iterator will be equal to end().
    constexpr iterator begin() const noexcept { return ptr; }

    /// Returns an iterator to the element following the last element of the span.
    /// This element acts as a placeholder; attempting to access it results in undefined behavior.
    constexpr iterator end() const noexcept { return ptr + len; }

    /// Returns a reverse iterator to the first element of the reversed span.
    /// It corresponds to the last element of the non-reversed span.
    /// If the span is empty, the returned iterator is equal to rend().
    constexpr reverse_iterator rbegin() const noexcept { return reverse_iterator(end()); }

    /// Returns a reverse iterator to the element following the last element of the reversed span.
    /// It corresponds to the element preceding the first element of the non-reversed span.
    /// This element acts as a placeholder, attempting to access it results in undefined behavior.
    constexpr reverse_iterator rend() const noexcept { return reverse_iterator(begin()); }

    /// Obtains a span that is a view over the first Count elements of this span.
    template <std::size_t Count>
    constexpr Span<element_type, Count> first() const {
        return Span<element_type, Count>(ptr);
    }

    /// Obtains a span that is a view over the first Count elements of this span.
    constexpr Span<element_type> first(size_type count) const { return Span<element_type>(ptr, count); }

    /// Obtains a span that is a view over the last Count elements of this span.
    template <std::size_t Count>
    constexpr Span<element_type, Count> last() const {
        return Span<element_type, Count>(ptr + (len - Count));
    }

    /// Obtains a span that is a view over the last count elements of this span.
    constexpr Span<element_type> last(size_type count) const { return Span<element_type>(ptr + (len - count), count); }

    /// Obtains a span that is a view over the Count elements of this span starting at offset Offset.
    /// If Count is DYNAMIC_EXTENT, the number of elements in the subspan is size() - offset.
    template <size_type Offset, size_type Count>
    constexpr Span<element_type, Count> subspan() const {
        return Span<element_type, Count>(ptr + Offset);
    }

    /// Obtains a span that is a view over the Count elements of this span starting at offset Offset.
    /// If Count is DYNAMIC_EXTENT, the number of elements in the subspan is size() - offset.
    constexpr Span<element_type> subspan(size_type offset, size_type count = DYNAMIC_EXTENT) const {
        return Span<element_type>(ptr + offset, count == DYNAMIC_EXTENT ? len - offset : count);
    }

private:
    pointer ptr{nullptr};
    size_type len{0};
};

}  // namespace libdnf

#endif
