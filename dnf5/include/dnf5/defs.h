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

#ifndef DNF5_DEFS_H
#define DNF5_DEFS_H

// Generic helper definitions for shared library support
#if defined _WIN32 || defined __CYGWIN__
#define DNF_SYMBOL_IMPORT __declspec(dllimport)
#define DNF_SYMBOL_EXPORT __declspec(dllexport)
#define DNF_SYMBOL_LOCAL
#else
#if __GNUC__ >= 4
#define DNF_SYMBOL_IMPORT __attribute__((visibility("default")))
#define DNF_SYMBOL_EXPORT __attribute__((visibility("default")))
#define DNF_SYMBOL_LOCAL  __attribute__((visibility("hidden")))
#else
#define DNF_SYMBOL_IMPORT
#define DNF_SYMBOL_EXPORT
#define DNF_SYMBOL_LOCAL
#endif
#endif

// Now define DNF_API, DNF_LOCAL and DNF_PLUGIN_API.
// DNF_API and DNF_PLUGIN_API are used for public API symbols. It either imports or exports the symbol.
// DNF_LOCAL is used for non-api symbols.

#ifdef DNF_BUILD_APPLICATION  // defined if we are building the dnf application
#define DNF_API DNF_SYMBOL_EXPORT
#else
#define DNF_API DNF_SYMBOL_IMPORT
#endif

#define DNF_LOCAL DNF_SYMBOL_LOCAL

// DNF exports its symbols but imports plugin symbols.
#ifdef DNF_BUILD_APPLICATION  // defined if we are building the dnf application
#define DNF_PLUGIN_API DNF_SYMBOL_IMPORT
#else
#define DNF_PLUGIN_API DNF_SYMBOL_EXPORT
#endif

#endif
