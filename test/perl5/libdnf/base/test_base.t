# Copyright (C) 2020 Red Hat, Inc.
#
# This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
#
# Libdnf is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Libdnf is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

use strict;
use warnings;
use Test::More;

use libdnf::base;

my $base = new libdnf::base::Base();
ok(defined $base, 'new libdnf::base::Base() returned something');
ok($base->isa('libdnf::base::Base'), "  and it's the right class" );
my $loger = $base->get_logger();
my $config = $base->get_config();
my $repo_sack = $base->get_rpm_repo_sack();
my $solv_sack = $base->get_rpm_solv_sack();

done_testing()
