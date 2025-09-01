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

use File::Spec::Functions 'catfile';
use Test::More;

use FindBin;
use lib "$FindBin::Bin/..";
use BaseTestCase;

my $USER_DATA = 25;

{
    package DownloadCallbacks;
    use base qw(libdnf5::repo::DownloadCallbacks);

    my $USER_CB_DATA = 5;

    sub new {
        my $class = shift;
        my $self = $class->SUPER::new(@_);
        $self->{start_cnt} = 0;
        $self->{progress_cnt} = 0;
        $self->{mirror_failure_cnt} = 0;
        $self->{end_cnt} = 0;
        $self->{end_status} = undef;
        $self->{end_msg} = undef;
        return bless($self, $class);
    }

    sub add_new_download {
        my ($self, $user_data, $description, $total_to_download) = @_;
        $self->{start_cnt}++;
        ::is($user_data, $USER_DATA);
        return $USER_CB_DATA;
    }

    sub end {
        my ($self, $user_cb_data, $status, $msg) = @_;
        $self->{end_cnt}++;
        ::is($user_cb_data, $USER_CB_DATA);
        $self->{end_status} = $status;
        $self->{end_msg} = $msg;
        return 0;
    }

    sub progress {
        my ($self, $user_cb_data, $total_to_download, $downloaded) = @_;
        $self->{progress_cnt}++;
        ::is($user_cb_data, $USER_CB_DATA);
        return 0;
    }

    sub mirror_failure {
        my ($self, $user_cb_data, $msg, $url, $metadata) = @_;
        $self->{mirror_failure_cnt}++;
        ::is($user_cb_data, $USER_CB_DATA);
        return 0;
    }
}

# test_file_downloader
{
    my $test = new BaseTestCase();

    my $source_file_path = catfile($BaseTestCase::PROJECT_SOURCE_DIR, "test/data/keys/key.pub");
    my $source_url = "file://" . $source_file_path;
    my $dest_file_path = catfile($test->{temp_dir}, "file_downloader.pub");

    my $dl_cbs = new DownloadCallbacks();
    $test->{base}->set_download_callbacks(new libdnf5::repo::DownloadCallbacksUniquePtr($dl_cbs));
    $dl_cbs = $test->{base}->get_download_callbacks();

    my $file_downloader = new libdnf5::repo::FileDownloader($test->{base});
    $file_downloader->add($source_url, $dest_file_path, $USER_DATA);
    $file_downloader->download();

    is($dl_cbs->{start_cnt}, 1);
    ok($dl_cbs->{progress_cnt} >= 1);
    is($dl_cbs->{mirror_failure_cnt}, 0);
    is($dl_cbs->{end_cnt}, 1);

    is($dl_cbs->{end_status}, $libdnf5::repo::DownloadCallbacks::TransferStatus_SUCCESSFUL);
    is($dl_cbs->{end_msg}, undef);
}

done_testing();
