# Copyright Contributors to the libdnf project.
# SPDX-License-Identifier: GPL-2.0-or-later
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

use Test::More;

use FindBin;
use lib "$FindBin::Bin/..";
use BaseTestCase;

{
    package PackageDownloadCallbacks;
    use base qw(libdnf5::repo::DownloadCallbacks);

    sub new {
        my $class = shift;
        my $self = $class->SUPER::new(@_);
        $self->{user_cb_data_container} = [];
        $self->{start_cnt} = 0;
        $self->{progress_cnt} = 0;
        $self->{mirror_failure_cnt} = 0;
        $self->{end_cnt} = 0;
        $self->{user_data_array} = [];
        $self->{user_cb_data_array} = [];
        $self->{end_status} = [];
        $self->{end_msg} = [];
        return bless($self, $class);
    }

    sub add_new_download {
        my ($self, $user_data, $description, $total_to_download) = @_;
        $self->{start_cnt}++;
        push(@{$self->{user_data_array}}, $user_data);
        my $user_cb_data = "Package: " . $description;
        push(@{$self->{user_cb_data_container}}, $user_cb_data);
        return scalar @{$self->{user_cb_data_container}} - 1;
    }

    sub end {
        my ($self, $user_cb_data, $status, $error_message) = @_;
        $self->{end_cnt}++;
        ::ok($user_cb_data>=0 && $user_cb_data<=1, "end: user_cb_data");
        push @{$self->{user_cb_data_array}}, $user_cb_data;
        push @{$self->{end_status}}, $status;
        push @{$self->{end_msg}}, $error_message;
        return 0;
    }

    sub progress {
        my ($self, $user_cb_data, $total_to_download, $downloaded) = @_;
        $self->{progress_cnt}++;
        ::ok($user_cb_data>=0 && $user_cb_data<=1, "progress: user_cb_data");
        return 0;
    }

    sub mirror_failure {
        my ($self, $user_cb_data, $msg, $url, $metadata) = @_;
        $self->{mirror_failure_cnt}++;
        ::ok($user_cb_data>=0 && $user_cb_data<=1, "mirror_failure: user_cb_data");
        return 0;
    }
}

my $test = new BaseTestCase();

my $repo = $test->add_repo_rpm("rpm-repo1");

my $query = new libdnf5::rpm::PackageQuery($test->{base});
$query->filter_name(["one"]);
$query->filter_arch(["noarch"]);
is($query->size(), 2, "number of packages in query");

my $downloader = new libdnf5::repo::PackageDownloader($test->{base});

my $cbs = new PackageDownloadCallbacks();
$test->{base}->set_download_callbacks(new libdnf5::repo::DownloadCallbacksUniquePtr($cbs));

my $user_data = 2;
my $it = $query->begin();
my $e = $query->end();
while ($it != $e) {
    $downloader->add($it->value(), $user_data);
    $user_data *= 5;
    $it->next();
}
$downloader->download();

$cbs = $test->{base}->get_download_callbacks();

is_deeply($cbs->{user_cb_data_container}, ["Package: one-0:1-1.noarch", "Package: one-0:2-1.noarch"]);

is($cbs->{start_cnt}, 2, "start_cnt");
is($cbs->{end_cnt}, 2, "end_cnt");
ok($cbs->{progress_cnt} >= 2, "progress_cnt");
is($cbs->{mirror_failure_cnt}, 0, "mirror_failure_cnt");

is_deeply($cbs->{user_data_array}, [2, 10], "user_data_array");

is_deeply($cbs->{user_cb_data_array}, [0, 1], "user_cb_data_array");
is_deeply($cbs->{end_status}, [$libdnf5::repo::DownloadCallbacks::TransferStatus_SUCCESSFUL,  $libdnf5::repo::DownloadCallbacks::TransferStatus_SUCCESSFUL]);
is_deeply($cbs->{end_status}, [0, 0]);

done_testing();
