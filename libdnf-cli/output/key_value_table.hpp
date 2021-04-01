/*
Copyright (C) 2021 Red Hat, Inc.

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


#ifndef LIBDNF_CLI_OUTPUT_KEY_VALUE_TABLE_HPP
#define LIBDNF_CLI_OUTPUT_KEY_VALUE_TABLE_HPP

#include <libsmartcols/libsmartcols.h>

#include <string>
#include <vector>


namespace libdnf::cli::output {


class KeyValueTable {
public:
    explicit KeyValueTable();
    ~KeyValueTable();
    void print();

protected:
    struct libscols_line * add_line(
        const char * key,
        const char * value,
        const char * color = nullptr,
        struct libscols_line * parent = nullptr
    );

    struct libscols_line * add_line(
        const char * key,
        const std::string & value,
        const char * color = nullptr,
        struct libscols_line * parent = nullptr
    );

    struct libscols_line * add_line(
        const char * key,
        const std::vector<std::string> & value,
        const char * color = nullptr,
        struct libscols_line * parent = nullptr
    );

    template<typename V>
    struct libscols_line * add_line(
        const char * key,
        V value,
        const char * color = nullptr,
        struct libscols_line * parent = nullptr
    ) {
        return add_line(key, std::to_string(value), color, parent);
    }

private:
    struct libscols_table * tb = nullptr;
};


}  // namespace libdnf::cli::output


#endif  // LIBDNF_CLI_OUTPUT_KEY_VALUE_TABLE_HPP
