# Copyright Contributors to the libdnf project.
#
# This file is part of libdnf: https://github.com/rpm-software-management/libdnf/
#
# Libdnf is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# Libdnf is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with libdnf.  If not, see <https://www.gnu.org/licenses/>.

use strict;
use warnings;
no if $] >= 5.010, warnings => qw(experimental::smartmatch);

use Test::More;
use Cwd qw(cwd);
use File::Temp qw(tempdir);
use File::Spec::Functions 'catfile';


use libdnf5::base;

my $base = new libdnf5::base::Base();

# Sets path to cache directory.
my $tmpdir = tempdir("libdnf5_perl5_unittest.XXXX", TMPDIR => 1, CLEANUP => 1);
$base->get_config()->get_installroot_option()->set($libdnf5::conf::Option::Priority_RUNTIME, $tmpdir."/installroot");
$base->get_config()->get_cachedir_option()->set($libdnf5::conf::Option::Priority_RUNTIME, $tmpdir."/cache");

# Prevent loading plugins from host
$base->get_config()->get_plugins_option()->set("False");

# Sets base internals according to configuration
$base->setup();

my $repo_sack = $base->get_repo_sack();

# Creates new repositories in the repo_sack
my $repoid = "repomd-repo1";
my $repo = $repo_sack->create_repo($repoid);

# Tunes repository configuration (baseurl is mandatory)
my $project_source_dir = $ENV{"PROJECT_SOURCE_DIR"};
my $repo_path = catfile($project_source_dir, "/test/data/repos-repomd/repomd-repo1/");
my $baseurl = "file://" . $repo_path;
my $repo_cfg = $repo->get_config();
$repo_cfg->get_baseurl_option()->set($libdnf5::conf::Option::Priority_RUNTIME, $baseurl);

# fetch repo metadata and load it
my $repos = new libdnf5::repo::RepoQuery($base);
$repos->filter_id($repoid);
$repo_sack->update_and_load_repos($repos);

#test_size()
{
    my $query = new libdnf5::rpm::PackageQuery($base);
    is($query->size(), 3);
}

my @nevras = ("CQRlib-1.1.1-4.fc29.src", "CQRlib-1.1.1-4.fc29.x86_64");
my @nevras_contains = ("CQRlib-1.1.1-4.fc29.src", "CQRlib-1.1.1-4.fc29.x86_64",
                       "CQRlib-devel-1.1.2-16.fc29.src", "CQRlib-devel-1.1.2-16.fc29.x86_64");
my @full_nevras = ("CQRlib-0:1.1.1-4.fc29.src", "CQRlib-0:1.1.1-4.fc29.x86_64",
                   "nodejs-1:5.12.1-1.fc29.src", "nodejs-1:5.12.1-1.fc29.x86_64");

# Test QueryCmp::EQ
{
    my $query = new libdnf5::rpm::PackageQuery($base);
    $query->filter_name(["pkg"]);
    is($query->size(), 1);

    my @expected = ("pkg-1.2-3.x86_64");
    my @actual = ();
    # TODO(dmach): implement iteration over query
    my $it = $query->begin();
    my $e = $query->end();
    while ($it != $e) {
        push @actual, $it->value()->get_nevra();
        $it->next();
    }
    ok(@actual ~~ @expected);
}

# Test QueryCmp::GLOB
{
    my $query = new libdnf5::rpm::PackageQuery($base);
    $query->filter_name(["pk*"], $libdnf5::common::QueryCmp_GLOB);
    is($query->size(), 2);

    my @expected = ("pkg-1.2-3.x86_64", "pkg-libs-1:1.3-4.x86_64");
    my @actual = ();
    # TODO(dmach): implement iteration over query
    my $it = $query->begin();
    my $e = $query->end();
    while ($it != $e) {
        #ok(exists($nevras_map{$it->value()->get_nevra()}));
        push @actual, $it->value()->get_nevra();
        $it->next();
    }
    ok(@actual ~~ @expected);
}

done_testing();
