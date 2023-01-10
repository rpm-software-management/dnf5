/*
Copyright Contributors to the libdnf project.

This file is part of libdnf: https://github.com/rpm-software-management/libdnf/

Libdnf is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Libdnf is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with libdnf.  If not, see <https://www.gnu.org/licenses/>.
*/


#include "transaction_test_base.hpp"


void TransactionTestBase::setUp() {
    temp_dir = std::make_unique<libdnf::utils::fs::TempDir>("libdnf_unittest");
}


void TransactionTestBase::tearDown() {}


std::unique_ptr<libdnf::Base> TransactionTestBase::new_base() {
    auto new_base = std::make_unique<libdnf::Base>();
    new_base->get_config().transaction_history_dir().set(temp_dir->get_path());
    return new_base;
}
