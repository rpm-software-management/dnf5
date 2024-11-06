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
    package PackageDownloadCallbacks;
    use base qw(libdnf5::repo::DownloadCallbacks);

    sub new {
        my $class = shift;
        my $self = $class->SUPER::new(@_);
        $self->{end_cnt} = 0;
        $self->{progress_cnt} = 0;
        $self->{mirror_failure_cnt} = 0;
        return bless($self, $class);
    }

    sub end {
        my ($self, $user_cb_data, $status, $error_message) = @_;
        $self->{end_cnt}++;
        return 0;
    }

    sub progress {
        my ($self, $user_cb_data, $total_to_download, $downloaded) = @_;
        $self->{progress_cnt}++;
        return 0;
    }

    sub mirror_failure {
        my ($self, $user_cb_data, $msg, $url, $metadata) = @_;
        my $self = shift;
        $self->{mirror_failure_cnt}++;
        return 0;
    }
}

my $test = new BaseTestCase();

my $repo = $test->add_repo_rpm("rpm-repo1");

my $query = new libdnf5::rpm::PackageQuery($test->{base});
$query->filter_name(["one"]);
$query->filter_arch(["noarch"]);
is($query->size(), 2);

my $downloader = new libdnf5::repo::PackageDownloader($test->{base});

my $cbs = new PackageDownloadCallbacks();
$test->{base}->set_download_callbacks(new libdnf5::repo::DownloadCallbacksUniquePtr($cbs));

my $it = $query->begin();
my $e = $query->end();
while ($it != $e) {
    $downloader->add($it->value());
    $it->next();
}
$downloader->download();

$cbs = $test->{base}->get_download_callbacks();

is($cbs->{end_cnt}, 2);
ok($cbs->{progress_cnt} >= 2);
is($cbs->{mirror_failure_cnt}, 0);

done_testing();
