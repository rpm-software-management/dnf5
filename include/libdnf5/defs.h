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

#ifndef LIBDNF5_DEFS_H
#define LIBDNF5_DEFS_H

// Generic helper definitions for shared library support
#if defined _WIN32 || defined __CYGWIN__
#define LIBDNF_SYMBOL_IMPORT __declspec(dllimport)
#define LIBDNF_SYMBOL_EXPORT __declspec(dllexport)
#define LIBDNF_SYMBOL_LOCAL
#else
#if __GNUC__ >= 4
#define LIBDNF_SYMBOL_IMPORT __attribute__((visibility("default")))
#define LIBDNF_SYMBOL_EXPORT __attribute__((visibility("default")))
#define LIBDNF_SYMBOL_LOCAL  __attribute__((visibility("hidden")))
#else
#define LIBDNF_SYMBOL_IMPORT
#define LIBDNF_SYMBOL_EXPORT
#define LIBDNF_SYMBOL_LOCAL
#endif
#endif

// Now define LIBDNF_API, LIBDNF_LOCAL and LIBDNF_PLUGIN_API.
// LIBDNF_API and LIBDNF_PLUGIN_API are used for public API symbols. It either imports or exports the symbol.
// LIBDNF_LOCAL is used for non-api symbols.

#ifdef LIBDNF_BUILD_LIBRARY  // defined if we are building the libdnf library (instead of using it)
#define LIBDNF_API LIBDNF_SYMBOL_EXPORT
#else
#define LIBDNF_API LIBDNF_SYMBOL_IMPORT
#endif

#define LIBDNF_LOCAL LIBDNF_SYMBOL_LOCAL

// LIBDNF exports its symbols but imports plugin symbols.
#ifdef LIBDNF_BUILD_LIBRARY  // defined if we are building the libdnf library (instead of using it)
#define LIBDNF_PLUGIN_API LIBDNF_SYMBOL_IMPORT
#else
#define LIBDNF_PLUGIN_API LIBDNF_SYMBOL_EXPORT
#endif

#endif
