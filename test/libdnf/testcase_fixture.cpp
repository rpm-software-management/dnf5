/*
Copyright (C) 2021 Red Hat, Inc.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "testcase_fixture.hpp"

#include <stdlib.h>

#include <map>
#include <string>
#include <vector>

extern char ** environ;

static bool environ_backuped{false};
static std::map<std::string, std::string> backup_environ;

void TestCaseFixture::setUp() {
    // Back up the current environment values.
    // Optimization. Do this only if the backup was not there before.
    std::string name;
    if (!environ_backuped) {
        for (char ** it = environ; *it; ++it) {
            char * name_end = *it;
            while (*name_end != '=' && *name_end != '\0') {
                ++name_end;
            }
            name.assign(*it, name_end);
            backup_environ[name] = getenv(name.c_str());
        }
        environ_backuped = true;
    }
}

void TestCaseFixture::tearDown() {
    // Remove extraneous variables from environment.
    std::string name;
    std::vector<std::string> vars_to_remove;
    for (char ** it = environ; *it; ++it) {
        char * name_end = *it;
        while (*name_end != '=' && *name_end != '\0') {
            ++name_end;
        }
        name.assign(*it, name_end);
        auto backup_it = backup_environ.find(name);
        if (backup_it == backup_environ.end()) {
            vars_to_remove.emplace_back(name);
        }
    }
    for (const auto & var : vars_to_remove) {
        unsetenv(var.c_str());
    }

    // Add the missing and synchronize the value of the existing environment variables.
    for (const auto & backup_var : backup_environ) {
        const char * cbackup_name = backup_var.first.c_str();
        const std::string & backup_value = backup_var.second;
        const char * cvalue = getenv(cbackup_name);
        if (!cvalue || cvalue != backup_value) {
            setenv(cbackup_name, backup_value.c_str(), 1);
        }
    }
}
