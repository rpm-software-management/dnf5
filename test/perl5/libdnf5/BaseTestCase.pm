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

package BaseTestCase;

use strict;
use warnings;

use File::Temp qw(tempdir);
use File::Spec::Functions 'catfile';

use libdnf5::base;
use libdnf5::repo;


our $PROJECT_BINARY_DIR = $ENV{"PROJECT_BINARY_DIR"};
our $PROJECT_SOURCE_DIR = $ENV{"PROJECT_SOURCE_DIR"};

sub new {
    my $class = shift;
    my $self = {};

    $self->{base} = new libdnf5::base::Base();

    $self->{temp_dir} = tempdir("libdnf5_perl5_unittest.XXXX", TMPDIR => 1, CLEANUP => 1);

    my $config = $self->{base}->get_config();
    $config->get_installroot_option()->set($libdnf5::conf::Option::Priority_RUNTIME, $self->{temp_dir}."/installroot");
    $config->get_cachedir_option()->set($libdnf5::conf::Option::Priority_RUNTIME, $self->{temp_dir}."/cache");
    $config->get_optional_metadata_types_option()->set($libdnf5::conf::Option::Priority_RUNTIME, $libdnf5::conf::OPTIONAL_METADATA_TYPES);

    # Prevent loading plugins from host
    $config->get_plugins_option()->set(0);

    my $vars = $self->{base}->get_vars()->get();
    $vars->set("arch", "x86_64");

    $self->{base}->setup();

    $self->{repo_sack} = $self->{base}->get_repo_sack();
    $self->{package_sack} = $self->{base}->get_rpm_package_sack();

    return bless ($self, $class);
}

sub tearDown {
    my $self = shift;
  #  shutil.rmtree(self.temp_dir)
}

sub _add_repo {
    # Add a repo from `repo_path`.
    my $self = shift;
    my $repoid = shift;
    my $repo_path = shift;
    my $load = shift // 1; # True is default

    my $repo = $self->{repo_sack}->create_repo($repoid);
    $repo->get_config()->get_baseurl_option()->set($libdnf5::conf::Option::Priority_RUNTIME, "file://".$repo_path);
    if ($load) {
        $self->{repo_sack}->load_repos($libdnf5::repo::Repo::Type_AVAILABLE);
    }

    return $repo
}

sub add_repo_repomd {
    # Add a repo from PROJECT_SOURCE_DIR/test/data/repos-repomd/<repoid>/repodata
    my $self = shift;
    my $repoid = shift;
    my $load = shift // 1; # True is default

    my $repo_path = catfile($PROJECT_SOURCE_DIR, "/test/data/repos-repomd", $repoid);
    return $self->_add_repo($repoid, $repo_path, $load)
}

sub add_repo_rpm {
    # Add a repo from PROJECT_BINARY_DIR/test/data/repos-rpm/<repoid>/repodata
    my $self = shift;
    my $repoid = shift;
    my $load = shift // 1; # True is default

    my $repo_path = catfile($PROJECT_BINARY_DIR, "test/data/repos-rpm", $repoid);
    return $self->_add_repo($repoid, $repo_path, $load)
}

sub add_repo_solv {
    # Add a repo from PROJECT_SOURCE_DIR/test/data/repos-solv/<repoid>.repo
    my $self = shift;
    my $repoid = shift;

    my $repo_path = catfile($PROJECT_SOURCE_DIR, "/test/data/repos-solv", $repoid.".repo");
    return $self->{repo_sack}->create_repo_from_libsolv_testcase($repoid, $repo_path);
}

1;
