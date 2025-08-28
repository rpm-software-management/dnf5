// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later

#ifndef LIBDNF5_CLI_DEFS_H
#define LIBDNF5_CLI_DEFS_H

// Generic helper definitions for shared library support
#if defined _WIN32 || defined __CYGWIN__
#define LIBDNF_CLI_SYMBOL_IMPORT __declspec(dllimport)
#define LIBDNF_CLI_SYMBOL_EXPORT __declspec(dllexport)
#define LIBDNF_CLI_SYMBOL_LOCAL
#else
#if __GNUC__ >= 4
#define LIBDNF_CLI_SYMBOL_IMPORT __attribute__((visibility("default")))
#define LIBDNF_CLI_SYMBOL_EXPORT __attribute__((visibility("default")))
#define LIBDNF_CLI_SYMBOL_LOCAL  __attribute__((visibility("hidden")))
#else
#define LIBDNF_CLI_SYMBOL_IMPORT
#define LIBDNF_CLI_SYMBOL_EXPORT
#define LIBDNF_CLI_SYMBOL_LOCAL
#endif
#endif

// Now we use the generic helper definitions above to define LIBDNF_CLI_API and LIBDNF_CLI_LOCAL.
// LIBDNF_CLI_API is used for the public API symbols. It either imports or exports the symbol.
// LIBDNF_CLI_LOCAL is used for non-api symbols.

#ifdef LIBDNF_CLI_BUILD_LIBRARY  // defined if we are building the libdnf-cli library (instead of using it)
#define LIBDNF_CLI_API LIBDNF_CLI_SYMBOL_EXPORT
#else
#define LIBDNF_CLI_API LIBDNF_CLI_SYMBOL_IMPORT
#endif

#define LIBDNF_CLI_LOCAL LIBDNF_CLI_SYMBOL_LOCAL

#endif
