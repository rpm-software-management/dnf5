Feature: dnf "config-manager" command - test "addrepo" subcommand


Background:
  Given I create file "/etc/yum.repos.d/repo1.repo" with
        """
        [repo1]
        name=repo1 test repository
        enabled=1
        baseurl=http://something1.com/os/
        """


Scenario: test "addrepo" from "baseurl", "enabled=1" is set by default
   When I execute dnf with args "config-manager addrepo --set=baseurl=http://something.com/os/"
   Then the exit code is 0
    And file "/etc/yum.repos.d/something.com_os_.repo" contents is
        """
        [something.com_os_]
        name=something.com_os_ - Created by dnf5 config-manager
        enabled=1
        baseurl=http://something.com/os/
        """


Scenario: test "addrepo" from "baseurl" with user defined repository id (--id=)
   When I execute dnf with args "config-manager addrepo --id=test --set=baseurl=http://something.com/os/"
   Then the exit code is 0
    And file "/etc/yum.repos.d/test.repo" contents is
        """
        [test]
        name=test - Created by dnf5 config-manager
        enabled=1
        baseurl=http://something.com/os/
        """


Scenario: test "addrepo" from "baseurl" with user defined destination file name (--save-filename=)
   When I execute dnf with args "config-manager addrepo --save-filename=test.repo --set=baseurl=http://something.com/os/"
   Then the exit code is 0
    And file "/etc/yum.repos.d/test.repo" contents is
        """
        [something.com_os_]
        name=something.com_os_ - Created by dnf5 config-manager
        enabled=1
        baseurl=http://something.com/os/
        """


Scenario: "addrepo" from "baseurl" with user defined destination file name, test adding the .repo extension to the filename
   When I execute dnf with args "config-manager addrepo --save-filename=test --set=baseurl=http://something.com/os/"
   Then the exit code is 0
    And file "/etc/yum.repos.d/test.repo" contents is
        """
        [something.com_os_]
        name=something.com_os_ - Created by dnf5 config-manager
        enabled=1
        baseurl=http://something.com/os/
        """


Scenario: when run addrepo without URL
   When I execute dnf with args "config-manager addrepo --id=test_repo --set=excludepkgs=iftop"
   Then the exit code is 2
    And stderr contains "One of --from-repofile=<URL>, --set=baseurl=<URL>, --set=mirrorlist=<URL>, --set=metalink=<URL> must be set to a non-empty URL"


Scenario: test "addrepo" from "baseurl", set more options
   When I execute dnf with args "config-manager addrepo --set=baseurl=http://something.com/os/ --set=gpgcheck=1 --set=metadata_expire=7d"
   Then the exit code is 0
    And file "/etc/yum.repos.d/something.com_os_.repo" contents is
        """
        [something.com_os_]
        name=something.com_os_ - Created by dnf5 config-manager
        enabled=1
        baseurl=http://something.com/os/
        gpgcheck=1
        metadata_expire=7d
        """


Scenario: test "addrepo" from "baseurl", set option multiple times with the same value is OK
   When I execute dnf with args "config-manager addrepo --set=baseurl=http://something.com/os/ --set=gpgcheck=1 --set=metadata_expire=7d --set=gpgcheck=1"
   Then the exit code is 0
    And file "/etc/yum.repos.d/something.com_os_.repo" contents is
        """
        [something.com_os_]
        name=something.com_os_ - Created by dnf5 config-manager
        enabled=1
        baseurl=http://something.com/os/
        gpgcheck=1
        metadata_expire=7d
        """


Scenario: test "addrepo" from "baseurl", set option more times with different value
   When I execute dnf with args "config-manager addrepo --set=baseurl=http://something.com/os/ --set=gpgcheck=1 --set=metadata_expire=7d --set=gpgcheck=0"
   Then the exit code is 1
    And stderr contains "Sets the "gpgcheck" option again with a different value: "1" != "0""


Scenario: test setting non-existent option
   When I execute dnf with args "config-manager addrepo --set=baseurl=http://something.com/os/ --set=nonexistent=1"
   Then the exit code is 1
    And stderr contains "Cannot set repository option "nonexistent=1": Option "nonexistent" not found"


Scenario: tests for setting an invalid value
   When I execute dnf with args "config-manager addrepo --set=baseurl=http://something.com/os/ --set=gpgcheck=XX"
   Then the exit code is 1
    And stderr contains "Cannot set repository option "gpgcheck=XX": Invalid boolean value "XX""


Scenario: test "addrepo" from "baseurl", destination directory does not exist
  Given I delete directory "/etc/yum.repos.d/"
   When I execute dnf with args "config-manager addrepo --id=test --set=baseurl=http://something.com/os/"
   Then the exit code is 1
    And stderr contains "Directory ".*/etc/yum.repos.d" does not exist. Add "--create-missing-dir" to create missing directories"


Scenario: test "addrepo" from "baseurl", destination directory does not exist, "--create-missing-dir" creates missing directory
  Given I delete directory "/etc/yum.repos.d/"
   When I execute dnf with args "config-manager addrepo --id=test --set=baseurl=http://something.com/os/ --create-missing-dir"
   Then the exit code is 0
    And file "/etc/yum.repos.d/test.repo" contents is
        """
        [test]
        name=test - Created by dnf5 config-manager
        enabled=1
        baseurl=http://something.com/os/
        """


Scenario: test "addrepo" from "baseurl", a file was found instead of destination directory
  Given I delete directory "/etc/yum.repos.d/"
    And I create file "/etc/yum.repos.d" with
    """
    """
   When I execute dnf with args "config-manager addrepo --id=test --set=baseurl=http://something.com/os/"
   Then the exit code is 1
    And stderr contains "The path ".*/etc/yum.repos.d" exists, but it is not a directory or a symlink to a directory"


Scenario: test "addrepo" from "baseurl", a symlink to non-existent object was found instead of destination directory
  Given I delete directory "/etc/yum.repos.d/"
    And I create symlink "/etc/yum.repos.d" to file "/non-exist"
   When I execute dnf with args "config-manager addrepo --id=test --set=baseurl=http://something.com/os/"
   Then the exit code is 1
    And stderr contains "The path ".*/etc/yum.repos.d" exists, but it is a symlink to a non-existent object"


# File "repo1.repo" with repository "repo1" was created in Background step
Scenario: test "addrepo" from "baseurl", destination repo file already exists
   When I execute dnf with args "config-manager addrepo --id=repo1 --set=baseurl=http://something.com/os/"
   Then the exit code is 1
    And stderr contains "File \".*/etc/yum.repos.d/repo1.repo\" already exists and configures repositories with IDs \"repo1\". Add "--add-or-replace" or "--overwrite""


# File "repo1.repo" with repository "repo1" was created in Background step
Scenario: test "addrepo" from "baseurl", repository id already exists
   When I execute dnf with args "config-manager addrepo --id=repo1 --set=baseurl=http://something.com/os/ --save-filename=another_repo1.repo"
   Then the exit code is 1
    And stderr contains "A repository with id \"repo1\" already configured in file: .*/etc/yum.repos.d/repo1.repo"


# File "repo1.repo" with repository "repo1" was created in Background step
Scenario: test "addrepo" from "baseurl", overwrite existing repo file (--overwrite), the repo id already exists but in the overwritten file so no problem
   When I execute dnf with args "config-manager addrepo --id=repo1 --set=baseurl=http://something.com/os/ --overwrite"
   Then the exit code is 0
    And file "/etc/yum.repos.d/repo1.repo" contents is
        """
        [repo1]
        name=repo1 - Created by dnf5 config-manager
        enabled=1
        baseurl=http://something.com/os/
        """


# File "repo1.repo" with repository "repo1" was created in Background step
Scenario: test "addrepo" from "baseurl", add repo to existing file (--add-or-replace)
   When I execute dnf with args "config-manager addrepo --id=test --set=baseurl=http://something.com/os/ --save-filename=repo1.repo --add-or-replace"
   Then the exit code is 0
    And file "/etc/yum.repos.d/repo1.repo" contents is
        """
        [repo1]
        name=repo1 test repository
        enabled=1
        baseurl=http://something1.com/os/
        [test]
        name=test - Created by dnf5 config-manager
        enabled=1
        baseurl=http://something.com/os/
        """


# File "repo1.repo" with repository "repo1" was created in Background step
Scenario: test "addrepo" from "baseurl", replace repo in existing file (--add-or-replace), if the new repo has the same id, it will replace the existing repo
   When I execute dnf with args "config-manager addrepo --id=repo1 --set=baseurl=http://something.com/os/ --add-or-replace"
   Then the exit code is 0
    And file "/etc/yum.repos.d/repo1.repo" contents is
        """
        [repo1]
        name=repo1 - Created by dnf5 config-manager
        enabled=1
        baseurl=http://something.com/os/
        """


Scenario: test "addrepo" from repofile
  Given I create file "/{context.dnf.tempdir}/tmp/test.repo" with
        """
        [test]
        name=repository file
        enabled=0
        baseurl=http://something.com/os/
        """
    And I execute dnf with args "config-manager addrepo --from-repofile={context.dnf.tempdir}/tmp/test.repo"
   Then the exit code is 0
    And file "/etc/yum.repos.d/test.repo" contents is
        """
        [test]
        name=repository file
        enabled=0
        baseurl=http://something.com/os/
        """


Scenario: test "addrepo" from repofile with comments and empty line
  Given I create file "/{context.dnf.tempdir}/tmp/test.repo" with
        """
        # Repository configuration file
        [test] # Test repo
        name=repository file

        enabled=0
        # URL to repository
        baseurl=http://something.com/os/
        """
    And I execute dnf with args "config-manager addrepo --from-repofile={context.dnf.tempdir}/tmp/test.repo"
   Then the exit code is 0
    And file "/etc/yum.repos.d/test.repo" contents is
        """
        # Repository configuration file
        [test] # Test repo
        name=repository file

        enabled=0
        # URL to repository
        baseurl=http://something.com/os/
        """


Scenario: "addrepo" from repofile, source file without .repo extension, adding the .repo extension to the destination filename
  Given I create file "/{context.dnf.tempdir}/tmp/test" with
        """
        [test]
        name=repository file
        enabled=0
        baseurl=http://something.com/os/
        """
    And I execute dnf with args "config-manager addrepo --from-repofile={context.dnf.tempdir}/tmp/test"
   Then the exit code is 0
    And file "/etc/yum.repos.d/test.repo" contents is
        """
        [test]
        name=repository file
        enabled=0
        baseurl=http://something.com/os/
        """


Scenario: test "addrepo" from repofile with user defined destination file name (--save-filename=)
  Given I create file "/{context.dnf.tempdir}/tmp/original.repo" with
        """
        [test]
        name=repository file
        enabled=0
        baseurl=http://something.com/os/
        """
   When I execute dnf with args "config-manager addrepo --from-repofile={context.dnf.tempdir}/tmp/original.repo --save-filename=test.repo"
   Then the exit code is 0
    And file "/etc/yum.repos.d/test.repo" contents is
        """
        [test]
        name=repository file
        enabled=0
        baseurl=http://something.com/os/
        """


Scenario: test "addrepo" from repofile with user defined destination file name (--save-filename=), adding the .repo extension to the destination filename
  Given I create file "/{context.dnf.tempdir}/tmp/original.repo" with
        """
        [test]
        name=repository file
        enabled=0
        baseurl=http://something.com/os/
        """
   When I execute dnf with args "config-manager addrepo --from-repofile={context.dnf.tempdir}/tmp/original.repo --save-filename=test"
   Then the exit code is 0
    And file "/etc/yum.repos.d/test.repo" contents is
        """
        [test]
        name=repository file
        enabled=0
        baseurl=http://something.com/os/
        """


Scenario: test "addrepo" from repofile, destination directory does not exist
  Given I delete directory "/etc/yum.repos.d/"
    And I create file "/{context.dnf.tempdir}/tmp/test.repo" with
        """
        [test]
        name=repository file
        enabled=0
        baseurl=http://something.com/os/
        """
   When I execute dnf with args "config-manager addrepo --from-repofile={context.dnf.tempdir}/tmp/test.repo"
   Then the exit code is 1
    And stderr contains "Directory ".*/etc/yum.repos.d" does not exist. Add "--create-missing-dir" to create missing directories"


Scenario: test "addrepo" from repofile, destination directory does not exist, "--create-missing-dir" creates missing directory
  Given I delete directory "/etc/yum.repos.d/"
    And I create file "/{context.dnf.tempdir}/tmp/test.repo" with
        """
        [test]
        name=repository file
        enabled=0
        baseurl=http://something.com/os/
        """
   When I execute dnf with args "config-manager addrepo --from-repofile={context.dnf.tempdir}/tmp/test.repo --create-missing-dir"
   Then the exit code is 0
    And file "/etc/yum.repos.d/test.repo" contents is
        """
        [test]
        name=repository file
        enabled=0
        baseurl=http://something.com/os/
        """


Scenario: test "addrepo" from repofile, a file was found instead of destination directory
  Given I delete directory "/etc/yum.repos.d/"
    And I create file "/etc/yum.repos.d" with
    """
    """
    And I create file "/{context.dnf.tempdir}/tmp/test.repo" with
        """
        [test]
        name=repository file
        enabled=0
        baseurl=http://something.com/os/
        """
   When I execute dnf with args "config-manager addrepo --from-repofile={context.dnf.tempdir}/tmp/test.repo"
   Then the exit code is 1
    And stderr contains "The path ".*/etc/yum.repos.d" exists, but it is not a directory or a symlink to a directory"


Scenario: test "addrepo" from repofile, a symlink to non-existent object was found instead of destination directory
  Given I delete directory "/etc/yum.repos.d/"
    And I create symlink "/etc/yum.repos.d" to file "/non-exist"
    And I create file "/{context.dnf.tempdir}/tmp/test.repo" with
        """
        [test]
        name=repository file
        enabled=0
        baseurl=http://something.com/os/
        """
   When I execute dnf with args "config-manager addrepo --from-repofile={context.dnf.tempdir}/tmp/test.repo"
   Then the exit code is 1
    And stderr contains "The path ".*/etc/yum.repos.d" exists, but it is a symlink to a non-existent object"


# File "repo1.repo" with repository "repo1" was created in Background step
Scenario: test "addrepo" from repofile, destination repo file already exists
  Given I create file "/{context.dnf.tempdir}/tmp/repo1.repo" with
        """
        [repo1]
        name=repository file
        enabled=0
        baseurl=http://something.com/os/
        """
   When I execute dnf with args "config-manager addrepo --from-repofile={context.dnf.tempdir}/tmp/repo1.repo"
   Then the exit code is 1
    And stderr contains "File \".*/etc/yum.repos.d/repo1.repo\" already exists and configures repositories with IDs \"repo1\". Add "--overwrite" to overwrite"


# File "repo1.repo" with repository "repo1" was created in Background step
Scenario: test "addrepo" from repofile, repository id already exists
  Given I create file "/{context.dnf.tempdir}/tmp/test.repo" with
        """
        [repo1]
        name=repository file
        enabled=0
        baseurl=http://something.com/os/
        """
   When I execute dnf with args "config-manager addrepo --from-repofile={context.dnf.tempdir}/tmp/test.repo"
   Then the exit code is 1
    And stderr contains "A repository with id \"repo1\" already configured in file: .*/etc/yum.repos.d/repo1.repo"


# File "repo1.repo" with repository "repo1" was created in Background step
Scenario: test "addrepo" from repofile, overwrite existing repo file (--overwrite), the repo id already exists but in the overwritten file so no problem
  Given I create file "/{context.dnf.tempdir}/tmp/repo1.repo" with
        """
        [repo1]
        name=repository file
        enabled=0
        baseurl=http://something.com/os/
        """
   When I execute dnf with args "config-manager addrepo --from-repofile={context.dnf.tempdir}/tmp/repo1.repo --overwrite"
   Then the exit code is 0
    And file "/etc/yum.repos.d/repo1.repo" contents is
        """
        [repo1]
        name=repository file
        enabled=0
        baseurl=http://something.com/os/
        """


Scenario: test "addrepo" from repofile defined by file:/// URL
  Given I create file "/{context.dnf.tempdir}/tmp/test.repo" with
        """
        [test]
        name=repository file
        enabled=0
        baseurl=http://something.com/os/
        """
    And I execute dnf with args "config-manager addrepo --from-repofile=file:///{context.dnf.tempdir}/tmp/test.repo"
   Then the exit code is 0
    And file "/etc/yum.repos.d/test.repo" contents is
        """
        [test]
        name=repository file
        enabled=0
        baseurl=http://something.com/os/
        """


Scenario: test "addrepo" from remote repofile (download from http)
  Given I create directory "/remotedir"
    And I create file "/remotedir/test.repo" with
        """
        [test]
        name=repository file
        enabled=0
        baseurl=http://something.com/os/
        """
    And I set up a http server for directory "/remotedir"
   When I execute dnf with args "config-manager addrepo --from-repofile=http://localhost:{context.dnf.ports[/remotedir]}/test.repo"
   Then the exit code is 0
    And file "/etc/yum.repos.d/test.repo" contents is
        """
        [test]
        name=repository file
        enabled=0
        baseurl=http://something.com/os/
        """
