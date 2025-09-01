// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later
//
// This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
//
// Libdnf is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 2.1 of the License, or
// (at your option) any later version.
//
// Libdnf is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

#ifndef LIBDNF5_COMMON_IMPL_PTR_HPP
#define LIBDNF5_COMMON_IMPL_PTR_HPP


namespace libdnf5 {

template <class T>
class ImplPtr {
public:
    /// Constructs an ImplPtr that owns nothing.
    constexpr ImplPtr() noexcept = default;

    /// Constructs an ImplPtr that takes ownership of `ptr`.
    explicit constexpr ImplPtr(T * ptr) noexcept { this->ptr = ptr; }

    /// Constructs an ImplPtr that owns newly created instance value initialized from `src` or
    /// owns nothing if `src` owns nothing.
    ImplPtr(const ImplPtr & src) : ptr(src.ptr ? new T(*src.ptr) : nullptr) {}

    /// Constructs an ImplPtr by transferring ownership from `src` to *this and stores the null pointer in `src`.
    constexpr ImplPtr(ImplPtr && src) noexcept : ptr(src.ptr) { src.ptr = nullptr; }

    /// Copies the value pointed to by `src`, not the pointer itself.
    /// If the destination owns nothing and `src` points to a value, a new instance of the value initialized from
    /// `src` is created. Does not do anything if both of them own nothing.
    ImplPtr & operator=(const ImplPtr & src) {
        if (ptr != src.ptr) {
            if (ptr) {
                if (src.ptr) {
                    *ptr = *src.ptr;
                } else {
                    delete ptr;
                    ptr = nullptr;
                }
            } else {
                ptr = new T(*src.ptr);
            }
        }
        return *this;
    }

    /// Transfers ownership from `src` to *this and stores the null pointer in `src`.
    ImplPtr & operator=(ImplPtr && src) noexcept {
        if (ptr != src.ptr) {
            delete ptr;
            ptr = src.ptr;
            src.ptr = nullptr;
        }
        return *this;
    }

    ~ImplPtr() { delete ptr; }

    constexpr T * operator->() noexcept { return ptr; }

    constexpr const T * operator->() const noexcept { return ptr; }

    constexpr T & operator*() noexcept { return *ptr; }

    constexpr const T & operator*() const noexcept { return *ptr; }

    constexpr T * get() noexcept { return ptr; }

    constexpr const T * get() const noexcept { return ptr; }

private:
    T * ptr{nullptr};  // object is owner of the pointer
};

}  // namespace libdnf5

#endif
