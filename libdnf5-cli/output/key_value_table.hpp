// Copyright Contributors to the DNF5 project.
// Copyright Contributors to the libdnf project.
// SPDX-License-Identifier: LGPL-2.1-or-later


#ifndef LIBDNF5_CLI_OUTPUT_KEY_VALUE_TABLE_HPP
#define LIBDNF5_CLI_OUTPUT_KEY_VALUE_TABLE_HPP

#include <libsmartcols/libsmartcols.h>

#include <string>
#include <vector>

namespace libdnf5::cli::output {

class KeyValueTable {
public:
    explicit KeyValueTable();
    ~KeyValueTable();
    void print();

    struct libscols_line * add_line(
        const char * key, const char * value, const char * color = nullptr, struct libscols_line * parent = nullptr);

    struct libscols_line * add_line(
        const char * key,
        const std::string & value,
        const char * color = nullptr,
        struct libscols_line * parent = nullptr);

    struct libscols_line * add_line(
        const char * key,
        const std::vector<std::string> & value,
        const char * color = nullptr,
        struct libscols_line * parent = nullptr);

    struct libscols_line * add_lines(
        const char * key,
        const std::vector<std::string> & values,
        const char * color = nullptr,
        struct libscols_line * parent = nullptr);

    void drop_line_if_no_children(struct libscols_line * line);

    template <typename V>
    struct libscols_line * add_line(
        const char * key, V value, const char * color = nullptr, struct libscols_line * parent = nullptr) {
        return add_line(key, std::to_string(value), color, parent);
    }

private:
    struct libscols_table * tb = nullptr;
};

}  // namespace libdnf5::cli::output

#endif  // LIBDNF5_CLI_OUTPUT_KEY_VALUE_TABLE_HPP
