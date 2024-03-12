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

#ifndef LIBDNF5_DEFS_H
#define LIBDNF5_DEFS_H

// Generic helper definitions for shared library support
#if defined _WIN32 || defined __CYGWIN__
#define LIBDNF_DLL_IMPORT __declspec(dllimport)
#define LIBDNF_DLL_EXPORT __declspec(dllexport)
#define LIBDNF_DLL_LOCAL
#else
#if __GNUC__ >= 4
#define LIBDNF_DLL_IMPORT __attribute__((visibility("default")))
#define LIBDNF_DLL_EXPORT __attribute__((visibility("default")))
#define LIBDNF_DLL_LOCAL  __attribute__((visibility("hidden")))
#else
#define LIBDNF_DLL_IMPORT
#define LIBDNF_DLL_EXPORT
#define LIBDNF_DLL_LOCAL
#endif
#endif

// Now we use the generic helper definitions above to define LIBDNF_API and LIBDNF_LOCAL.
// LIBDNF_API is used for the public API symbols. It either DLL imports or DLL exports (or does nothing for static build)
// LIBDNF_LOCAL is used for non-api symbols.

#ifdef LIBDNF_DLL          // defined if LIBDNF is compiled as a DLL
#ifdef LIBDNF_DLL_EXPORTS  // defined if we are building the LIBDNF DLL (instead of using it)
#define LIBDNF_API LIBDNF_DLL_EXPORT
#else
#define LIBDNF_API LIBDNF_DLL_IMPORT
#endif  // LIBDNF_DLL_EXPORTS
#define LIBDNF_LOCAL LIBDNF_DLL_LOCAL
#else  // LIBDNF_DLL is not defined: this means LIBDNF is a static lib.
#define LIBDNF_API
#define LIBDNF_LOCAL
#endif  // LIBDNF_DLL

#endif
