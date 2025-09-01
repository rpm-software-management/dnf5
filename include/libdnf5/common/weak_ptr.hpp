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

#ifndef LIBDNF5_UTILS_WEAK_PTR_HPP
#define LIBDNF5_UTILS_WEAK_PTR_HPP

#include "exception.hpp"

#include <mutex>
#include <unordered_set>


namespace libdnf5 {

template <typename TPtr, bool ptr_owner>
struct WeakPtr;


/// WeakPtrGuard is a resource guard. WeakPtr instances register itself to the resource guard.
/// And the resource guard invalidates the registered WeakPtrs when the resource is unusable
/// (eg. its dependecny was released).
/// Note on thread safety:
/// Destroying the WeakPtrGuard while simultaneously using its WeakPtrs in another thread is not safe
/// and can still trigger a race condition.
template <typename TPtr, bool weak_ptr_is_owner>
struct WeakPtrGuard {
public:
    using TWeakPtr = WeakPtr<TPtr, weak_ptr_is_owner>;

    WeakPtrGuard() = default;
    WeakPtrGuard(const WeakPtrGuard &) = delete;
    WeakPtrGuard(WeakPtrGuard && src) noexcept = delete;
    ~WeakPtrGuard() { clear(); }

    WeakPtrGuard & operator=(const WeakPtrGuard & src) = delete;
    WeakPtrGuard & operator=(WeakPtrGuard && src) noexcept = delete;

    /// Returns true if the guard is empty, false otherwise.
    bool empty() const noexcept {
        std::lock_guard<std::mutex> guard(registered_weak_ptrs_mutex);
        return registered_weak_ptrs.empty();
    }

    /// Returns the number of registered weak pointers.
    size_t size() const noexcept {
        std::lock_guard<std::mutex> guard(registered_weak_ptrs_mutex);
        return registered_weak_ptrs.size();
    }

    /// Deregisters and invalidates all registered weak pointers. After this call, size() returns zero.
    void clear() noexcept {
        std::lock_guard<std::mutex> guard(registered_weak_ptrs_mutex);
        for (auto it : registered_weak_ptrs) {
            it->invalidate_guard();
        }
        registered_weak_ptrs.clear();
    }

private:
    friend TWeakPtr;
    void register_ptr(TWeakPtr * weak_ptr) {
        std::lock_guard<std::mutex> guard(registered_weak_ptrs_mutex);
        registered_weak_ptrs.insert(weak_ptr);
    }
    void unregister_ptr(TWeakPtr * weak_ptr) noexcept {
        std::lock_guard<std::mutex> guard(registered_weak_ptrs_mutex);
        registered_weak_ptrs.erase(weak_ptr);
    }
    std::unordered_set<TWeakPtr *> registered_weak_ptrs;
    mutable std::mutex registered_weak_ptrs_mutex;
};


/// WeakPtr is a "smart" pointer. It contains a pointer to resource and to guard of resource.
/// WeakPtr pointer can be owner of the resource. However, the resource itself may depend on another resource.
/// WeakPtr registers/unregisters itself at the guard of resource. And the resource guard invalidates
/// the registered WeakPtrs when the resource is unusable (eg. its dependecny was released).
/// Note on thread safety:
/// It is safe to create, access and destroy WeakPtrs in multiple threads simultaneously.
template <typename TPtr, bool ptr_owner>
struct WeakPtr {
public:
    using TWeakPtrGuard = WeakPtrGuard<TPtr, ptr_owner>;

    WeakPtr() : ptr(nullptr), guard(nullptr) {}

    WeakPtr(TPtr * ptr, TWeakPtrGuard * guard) : ptr(ptr), guard(guard) {
        libdnf_assert(guard != nullptr, "When initializing WeakPtr with a pointer, guard cannot be nullptr");
        guard->register_ptr(this);
    }

    // TODO(jrohel): Want we to allow copying invalid WeakPtr?
    WeakPtr(const WeakPtr & src) : guard(src.guard) {
        if constexpr (ptr_owner) {
            ptr = src.ptr ? new TPtr(*src.ptr) : nullptr;
        } else {
            ptr = src.ptr;
        }
        if (is_valid()) {
            guard->register_ptr(this);
        }
    }

    // TODO(jrohel): Want we to allow moving invalid WeakPtr?
    template <typename T = TPtr, typename std::enable_if<sizeof(T) && ptr_owner, int>::type = 0>
    WeakPtr(WeakPtr && src) : guard(src.guard) {
        if (src.is_valid()) {
            src.guard->unregister_ptr(&src);
        }
        src.guard = nullptr;
        ptr = src.ptr;
        src.ptr = nullptr;
        if (is_valid()) {
            guard->register_ptr(this);
        }
    }

    ~WeakPtr() {
        if (is_valid()) {
            guard->unregister_ptr(this);
        }
        if constexpr (ptr_owner) {
            delete ptr;
        }
    }

    // TODO(jrohel): Want we to allow copying invalid WeakPtr?
    WeakPtr & operator=(const WeakPtr & src) {
        if (guard == src.guard) {
            if (this == &src) {
                return *this;
            }
            if constexpr (ptr_owner) {
                delete ptr;
                ptr = src.ptr ? new TPtr(*src.ptr) : nullptr;
            } else {
                ptr = src.ptr;
            }
        } else {
            if (is_valid()) {
                guard->unregister_ptr(this);
            }
            guard = src.guard;
            if constexpr (ptr_owner) {
                delete ptr;
                ptr = src.ptr ? new TPtr(*src.ptr) : nullptr;
            } else {
                ptr = src.ptr;
            }
            if (is_valid()) {
                guard->register_ptr(this);
            }
        }
        return *this;
    }

    // TODO(jrohel): Want we to allow moving invalid WeakPtr?
    template <typename T = TPtr, typename std::enable_if<sizeof(T) && ptr_owner, int>::type = 0>
    WeakPtr & operator=(WeakPtr && src) {
        if (guard == src.guard) {
            if (this == &src) {
                return *this;
            }
            if (src.is_valid()) {
                src.guard->unregister_ptr(&src);
            }
            src.guard = nullptr;
            delete ptr;
            ptr = src.ptr;
            src.ptr = nullptr;
        } else {
            if (is_valid()) {
                guard->unregister_ptr(this);
            }
            if (src.is_valid()) {
                src.guard->unregister_ptr(&src);
            }
            guard = src.guard;
            src.guard = nullptr;
            delete ptr;
            ptr = src.ptr;
            src.ptr = nullptr;
            if (is_valid()) {
                guard->register_ptr(this);
            }
        }
        return *this;
    }

    /// Provides access to the managed object. Generates exception if object is not valid.
    TPtr * operator->() const {
        libdnf_assert(is_valid(), "Dereferencing an invalidated WeakPtr");
        return ptr;
    }

    /// Returns a pointer to the managed object. Generates exception if object is not valid.
    TPtr * get() const {
        libdnf_assert(is_valid(), "Dereferencing an invalidated WeakPtr");
        return ptr;
    }

    /// Checks if managed object is valid.
    bool is_valid() const noexcept { return guard; }

    /// Checks if the other WeakPtr instance has the same WeakPtrGuard.
    bool has_same_guard(const WeakPtr & other) const noexcept { return guard == other.guard; }

    TPtr & operator*() const { return *get(); }
    bool operator==(const WeakPtr & other) const { return ptr == other.ptr; }
    bool operator!=(const WeakPtr & other) const { return ptr != other.ptr; }
    bool operator<(const WeakPtr & other) const { return ptr < other.ptr; }
    bool operator>(const WeakPtr & other) const { return ptr > other.ptr; }
    bool operator<=(const WeakPtr & other) const { return ptr <= other.ptr; }
    bool operator>=(const WeakPtr & other) const { return ptr >= other.ptr; }

private:
    friend TWeakPtrGuard;
    void invalidate_guard() noexcept { guard = nullptr; }

    TPtr * ptr;
    TWeakPtrGuard * guard;
};

}  // namespace libdnf5

#endif
