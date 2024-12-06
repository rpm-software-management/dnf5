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

use Test::More;

use FindBin;
use lib "$FindBin::Bin/..";
use BaseTestCase;

{
    package DownloadCallbacks;
    use base qw(libdnf5::repo::DownloadCallbacks);

    sub new {
        my $class = shift;
        my $self = $class->SUPER::new(@_);
        $self->{last_user_data} = undef;
        $self->{start_cnt} = 0;
        $self->{end_cnt} = 0;
        $self->{end_error_message} = undef;
        $self->{fastest_mirror_cnt} = 0;
        $self->{handle_mirror_failure_cnt} = 0;
        return bless($self, $class);
    }

    sub add_new_download {
        my ($self, $user_data, $description, $total_to_download) = @_;
        $self->{start_cnt}++;
        $self->{last_user_data} = $user_data;
        return 0;
    }

    sub end {
        my ($self, $user_cb_data, $status, $error_message) = @_;
        $self->{end_cnt}++;
        $self->{end_error_message} = $error_message;
        return 0;
    }

    sub fastest_mirror {
        my ($self, $user_cb_data, $stage, $ptr) = @_;
        $self->{fastest_mirror_cnt}++;
    }

    sub handle_mirror_failure {
        my ($self, $user_cb_data, $msg, $url, $metadata) = @_;
        $self->{handle_mirror_failure_cnt}++;
        return 0;
    }
}

{
    package RepoCallbacks;
    use base qw(libdnf5::repo::RepoCallbacks);

    sub new {
        my $class = shift;
        my $self = $class->SUPER::new(@_);
        $self->{repokey_import_cnt} = 0;
        return bless($self, $class);
    }

    sub repokey_import {
        my ($self, $id, $user_id, $fingerprint, $url, $timestamp) = @_;
        $self->{repokey_import_cnt}++;
        return 1;
    }
}

# test_load_repo
{
    my $test = new BaseTestCase();

    my $repoid = "repomd-repo1";
    my $repo = $test->add_repo_repomd($repoid, 0);

    my $USER_DATA = 25;
    $repo->set_user_data($USER_DATA);
    is($repo->get_user_data(), $USER_DATA);

    my $dl_cbs = new DownloadCallbacks();
    $test->{base}->set_download_callbacks(new libdnf5::repo::DownloadCallbacksUniquePtr($dl_cbs));
    $dl_cbs = $test->{base}->get_download_callbacks();

    my $cbs = new RepoCallbacks();
    $repo->set_callbacks(new libdnf5::repo::RepoCallbacksUniquePtr($cbs));

    $test->{repo_sack}->load_repos($libdnf5::repo::Repo::Type_AVAILABLE);

    is($dl_cbs->{last_user_data}, $USER_DATA);

    is($dl_cbs->{start_cnt}, 1);
    is($dl_cbs->{end_cnt}, 1);
    is($dl_cbs->{end_error_message}, undef);

    is($dl_cbs->{fastest_mirror_cnt}, 0);
    is($dl_cbs->{handle_mirror_failure_cnt}, 0);
    is($cbs->{repokey_import_cnt}, 0);
}

# test_load_repo_overload
{
    my $test = new BaseTestCase();

    my $repoid = "repomd-repo1";
    my $repo = $test->add_repo_repomd($repoid, 0);

    my $dl_cbs = new DownloadCallbacks();
    $test->{base}->set_download_callbacks(new libdnf5::repo::DownloadCallbacksUniquePtr($dl_cbs));
    $dl_cbs = $test->{base}->get_download_callbacks();

    my $cbs = new RepoCallbacks();
    $repo->set_callbacks(new libdnf5::repo::RepoCallbacksUniquePtr($cbs));

    $test->{repo_sack}->load_repos();

    is($dl_cbs->{end_cnt}, 1);
    is($dl_cbs->{end_error_message}, undef);

    is($dl_cbs->{fastest_mirror_cnt}, 0);
    is($dl_cbs->{handle_mirror_failure_cnt}, 0);
    is($cbs->{repokey_import_cnt}, 0);
}

done_testing();
