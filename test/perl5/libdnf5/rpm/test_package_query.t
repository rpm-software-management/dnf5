# Copyright Contributors to the libdnf project.
# SPDX-License-Identifier: GPL-2.0-or-later

use strict;
use warnings;

use Test::More;

use FindBin;
use lib "$FindBin::Bin/..";
use BaseTestCase;


# Create an instance of BaseTestCase, it will be shared for all tests
my $test = new BaseTestCase();
my $repoid = "repomd-repo1";
my $repo = $test->add_repo_repomd($repoid);

# test_size()
{
    my $query = new libdnf5::rpm::PackageQuery($test->{base});
    is($query->size(), 3);
}

# Test QueryCmp::EQ
{
    my $query = new libdnf5::rpm::PackageQuery($test->{base});
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
    my $query = new libdnf5::rpm::PackageQuery($test->{base});
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
