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

use Cwd qw(cwd);
use File::Spec::Functions 'catfile';

use libdnf::base;

my $base = new libdnf::base::Base();

# Sets path to cache directory.
my $cwd = cwd;
$base->get_config()->cachedir()->set($libdnf::conf::Option::Priority_RUNTIME, $cwd);

my $repo_sack = new libdnf::rpm::RepoSack($base);
my $sack = new libdnf::rpm::SolvSack($base);

# Creates new repositories in the repo_sack
my $repo = $repo_sack->new_repo("dnf-ci-fedora");

# Tunes repositotory configuration (baseurl is mandatory)
my $repo_path = catfile(cwd, "../../../test/libdnf/rpm/repos-data/dnf-ci-fedora/");
my $baseurl = "file://" . $repo_path;
my $repo_cfg = $repo->get_config();
$repo_cfg->baseurl()->set($libdnf::conf::Option::Priority_RUNTIME, $baseurl);

# Loads repository into rpm::Repo.
$repo->load();

# Loads rpm::Repo into rpm::SolvSack
$sack->load_repo($repo->get(), $libdnf::rpm::SolvSack::LoadRepoFlags_NONE);

#test_size()
{
    my $query = new libdnf::rpm::SolvQuery($sack);
    is($query->size(), 291);
}

my @nevras = ("CQRlib-1.1.1-4.fc29.src", "CQRlib-1.1.1-4.fc29.x86_64");
my @nevras_contains = ("CQRlib-1.1.1-4.fc29.src", "CQRlib-1.1.1-4.fc29.x86_64",
                       "CQRlib-devel-1.1.2-16.fc29.src", "CQRlib-devel-1.1.2-16.fc29.x86_64");
my @full_nevras = ("CQRlib-0:1.1.1-4.fc29.src", "CQRlib-0:1.1.1-4.fc29.x86_64",
                   "nodejs-1:5.12.1-1.fc29.src", "nodejs-1:5.12.1-1.fc29.x86_64");

# Test QueryCmp::EQ
{
    my $query = new libdnf::rpm::SolvQuery($sack);
    my $names = ["CQRlib"];
    $query->ifilter_name($libdnf::common::QueryCmp_EQ, new libdnf::common::VectorString($names));
    is($query->size(), 2);
    my $pset = $query->get_package_set();
    is($pset->size(), 2);
    my $it = $pset->begin();
    my $e = $pset->end();
    my %nevras_map = map { $_ => 1 } @nevras;
    while ($it != $e) {
        ok(exists($nevras_map{$it->value()->get_nevra()}));
        $it->next();
    }
}

# Test QueryCmp::GLOB
{
    my $query2 = new libdnf::rpm::SolvQuery($sack);
    my $names2 = ["CQ?lib"];
    $query2->ifilter_name($libdnf::common::QueryCmp_GLOB, new libdnf::common::VectorString($names2));
    is($query2->size(), 2);
    my $pset2 = $query2->get_package_set();
    my $it = $pset2->begin();
    my $e = $pset2->end();
    my %nevras_map = map { $_ => 1 } @nevras;
    while ($it != $e) {
        ok(exists($nevras_map{$it->value()->get_nevra()}));
        $it->next();
    }
}

done_testing();
