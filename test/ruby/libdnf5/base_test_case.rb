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


require 'test/unit'
require 'tmpdir'
include Test::Unit::Assertions

require 'libdnf5/base'
require 'libdnf5/rpm'


PROJECT_BINARY_DIR = ENV["PROJECT_BINARY_DIR"]
PROJECT_SOURCE_DIR = ENV["PROJECT_SOURCE_DIR"]


class BaseTestCase < Test::Unit::TestCase
    def setup()
        @base = Libdnf5::Base::Base.new()

        @temp_dir = Dir.mktmpdir("libdnf5_ruby_unittest.")

        @base.get_config().get_installroot_option().set(File.join(@temp_dir, "installroot"))
        @base.get_config().get_cachedir_option().set(File.join(@temp_dir, "cache"))

        # Prevent loading of plugins from host
        @base.get_config().get_plugins_option().set(false)

        # Sets Base internals according to configuration
        @base.setup()

        @repo_sack = @base.get_repo_sack()
        @package_sack = @base.get_rpm_package_sack()
    end

    def teardown()
        FileUtils.remove_entry(@temp_dir)
    end

    # Add a repo from `repo_path`.
    def _add_repo(repoid, repo_path, load=true)
        repo = @repo_sack.create_repo(repoid)
        repo.get_config().get_baseurl_option().set("file://" + repo_path)

        if load
          @repo_sack.load_repos(Libdnf5::Repo::Repo::Type_AVAILABLE)
        end

        return repo
    end

    # Add a repo from PROJECT_SOURCE_DIR/test/data/repos-repomd/<repoid>/repodata
    def add_repo_repomd(repoid, load=true)
        repo_path = File.join(PROJECT_SOURCE_DIR, "test/data/repos-repomd", repoid)
        return _add_repo(repoid, repo_path, load)
    end

    # Add a repo from PROJECT_BINARY_DIR/test/data/repos-rpm/<repoid>/repodata
    def add_repo_rpm(repoid, load=true)
        repo_path = File.join(PROJECT_BINARY_DIR, "test/data/repos-rpm", repoid)
        return _add_repo(repoid, repo_path, load)
    end

    # Add a repo from PROJECT_SOURCE_DIR/test/data/repos-solv/<repoid>.repo
    def add_repo_solv(repoid)
        repo_path = File.join(PROJECT_SOURCE_DIR, "test/data/repos-solv", repoid + ".repo")
        return @repo_sack.create_repo_from_libsolv_testcase(repoid, repo_path)
    end
end
