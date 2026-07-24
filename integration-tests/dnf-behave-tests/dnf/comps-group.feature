Feature: Testing groups

# DNF-CI-Testgroup structure:
#   mandatory: filesystem (requires setup)
#   default: lame (requires lame-libs)
#   optional: flac
#   conditional: wget, requires filesystem-content

# dnf5 currently supports only group ids as a <group spec>. Names are not supported
# https://github.com/rpm-software-management/dnf5/issues/1599
@xfail
Scenario: Install and remove group using group name
  Given I use repository "dnf-ci-thirdparty"
    And I use repository "dnf-ci-fedora"
   When I execute dnf with args "group install DNF-CI-Testgroup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install-group | filesystem-0:3.9-2.fc29.x86_64    |
        | install-group | lame-0:3.100-4.fc29.x86_64        |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch      |
        | install-dep   | lame-libs-0:3.100-4.fc29.x86_64   |
        | group-install | DNF-CI-Testgroup                  |
   When I execute dnf with args "group remove DNF-CI-Testgroup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | remove        | filesystem-0:3.9-2.fc29.x86_64    |
        | remove        | lame-0:3.100-4.fc29.x86_64        |
        | remove-unused | setup-0:2.12.1-1.fc29.noarch      |
        | remove-unused | lame-libs-0:3.100-4.fc29.x86_64   |
        | group-remove  | DNF-CI-Testgroup                  |


# The rest of scenarios use a group id to specify the group.
# This way scenarios can be used for both dnf4 and dnf5 testing.

Scenario: Install and remove group
  Given I use repository "dnf-ci-thirdparty"
    And I use repository "dnf-ci-fedora"
   When I execute dnf with args "group install dnf-ci-testgroup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install-group | filesystem-0:3.9-2.fc29.x86_64    |
        | install-group | lame-0:3.100-4.fc29.x86_64        |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch      |
        | install-dep   | lame-libs-0:3.100-4.fc29.x86_64   |
        | group-install | DNF-CI-Testgroup                  |
   When I execute dnf with args "group remove dnf-ci-testgroup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | remove        | filesystem-0:3.9-2.fc29.x86_64    |
        | remove        | lame-0:3.100-4.fc29.x86_64        |
        | remove-unused | setup-0:2.12.1-1.fc29.noarch      |
        | remove-unused | lame-libs-0:3.100-4.fc29.x86_64   |
        | group-remove  | DNF-CI-Testgroup                  |


Scenario: Install a group that is already installed
  Given I use repository "dnf-ci-thirdparty"
    And I use repository "dnf-ci-fedora"
   When I execute dnf with args "group install dnf-ci-testgroup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install-group | filesystem-0:3.9-2.fc29.x86_64    |
        | install-group | lame-0:3.100-4.fc29.x86_64        |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch      |
        | install-dep   | lame-libs-0:3.100-4.fc29.x86_64   |
        | group-install | DNF-CI-Testgroup                  |
   When I execute dnf with args "group install dnf-ci-testgroup"
   Then the exit code is 0
    And Transaction is empty


Scenario: Install and remove group with excluded package
  Given I use repository "dnf-ci-thirdparty"
    And I use repository "dnf-ci-fedora"
   When I execute dnf with args "group install --exclude=lame dnf-ci-testgroup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install-group | filesystem-0:3.9-2.fc29.x86_64    |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch      |
        | group-install | DNF-CI-Testgroup                  |
   When I execute dnf with args "group remove dnf-ci-testgroup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | remove        | filesystem-0:3.9-2.fc29.x86_64    |
        | remove-unused | setup-0:2.12.1-1.fc29.noarch      |
        | group-remove  | DNF-CI-Testgroup                  |


@bz1707624
Scenario: Install installed group when group is not available
  Given I use repository "dnf-ci-thirdparty"
    And I use repository "dnf-ci-fedora"
   When I execute dnf with args "group install --exclude=lame dnf-ci-testgroup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install-group | filesystem-0:3.9-2.fc29.x86_64    |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch      |
        | group-install | DNF-CI-Testgroup                  |
   When I execute dnf with args "group install --disable-repo=dnf-ci-thirdparty dnf-ci-testgroup"
   Then the exit code is 1
    And stderr contains lines
    """
    Failed to resolve the transaction:
    No match for argument: dnf-ci-testgroup
    """
    And stderr does not contain "ValueError"


Scenario: Install and remove group with excluded package dependency
  Given I use repository "dnf-ci-thirdparty"
    And I use repository "dnf-ci-fedora"
   When I execute dnf with args "group install --exclude=setup dnf-ci-testgroup"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        Problem: package filesystem-3.9-2.fc29.x86_64 from dnf-ci-fedora requires setup, but none of the providers can be installed
          - conflicting requests
          - package setup-2.12.1-1.fc29.noarch from dnf-ci-fedora is filtered out by exclude filtering
        You can try to add to command line:
          --skip-broken to skip uninstallable packages
        """


@bz1673851
Scenario: Install condidional package if required package is about to be installed
  Given I use repository "dnf-ci-thirdparty"
    And I use repository "dnf-ci-fedora"
   When I execute dnf with args "install @dnf-ci-testgroup filesystem-content"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | filesystem-content-0:3.9-2.fc29.x86_64    |
        | group-install | DNF-CI-Testgroup                          |
        | install-group | filesystem-0:3.9-2.fc29.x86_64            |
        | install-group | lame-0:3.100-4.fc29.x86_64                |
        | install-group | wget-0:1.19.5-5.fc29.x86_64               |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch              |
        | install-dep   | lame-libs-0:3.100-4.fc29.x86_64           |
   When I execute dnf with args "group remove dnf-ci-testgroup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | group-remove  | DNF-CI-Testgroup                          |
        | remove        | filesystem-0:3.9-2.fc29.x86_64            |
        | remove        | lame-0:3.100-4.fc29.x86_64                |
        | remove        | wget-0:1.19.5-5.fc29.x86_64               |
        | remove-unused | lame-libs-0:3.100-4.fc29.x86_64           |
        | remove-unused | setup-0:2.12.1-1.fc29.noarch              |


@bz1673851
Scenario: Install condidional package if required package has been installed
  Given I use repository "dnf-ci-thirdparty"
    And I use repository "dnf-ci-fedora"
   When I execute dnf with args "install filesystem-content"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | filesystem-content-0:3.9-2.fc29.x86_64    |
   When I execute dnf with args "group install dnf-ci-testgroup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | group-install | DNF-CI-Testgroup                          |
        | install-group | filesystem-0:3.9-2.fc29.x86_64            |
        | install-group | lame-0:3.100-4.fc29.x86_64                |
        | install-group | wget-0:1.19.5-5.fc29.x86_64               |
        | install-dep   | lame-libs-0:3.100-4.fc29.x86_64           |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch              |


Scenario: Group remove does not remove packages required by user installed packages
  Given I use repository "dnf-ci-thirdparty"
    And I use repository "dnf-ci-fedora"
   When I execute dnf with args "group install dnf-ci-testgroup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install-group | filesystem-0:3.9-2.fc29.x86_64            |
        | install-group | lame-0:3.100-4.fc29.x86_64                |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch              |
        | install-dep   | lame-libs-0:3.100-4.fc29.x86_64           |
        | group-install | DNF-CI-Testgroup                          |
   When I execute dnf with args "install basesystem"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | basesystem-0:11-6.fc29.noarch             |
        # setup and filesystem packages should be kept because they are required by
        # userinstalled basesystem
   When I execute dnf with args "group remove dnf-ci-testgroup"
   Then the exit code is 0
    And Transaction is following
        | Action            | Package                                   |
        | remove            | lame-0:3.100-4.fc29.x86_64                |
        | remove-unused     | lame-libs-0:3.100-4.fc29.x86_64           |
        | group-remove      | DNF-CI-Testgroup                          |
        | changing-reason   | filesystem-0:3.9-2.fc29.x86_64            |
        | unchanged         | setup-0:2.12.1-1.fc29.noarch              |
   When I execute dnf with args "remove basesystem"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | remove        | basesystem-0:11-6.fc29.noarch             |
        | remove-unused | filesystem-0:3.9-2.fc29.x86_64            |
        | remove-unused | setup-0:2.12.1-1.fc29.noarch              |


Scenario: Group remove does not remove user installed packages
  Given I use repository "dnf-ci-thirdparty"
    And I use repository "dnf-ci-fedora"
   When I execute dnf with args "install filesystem"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | filesystem-0:3.9-2.fc29.x86_64            |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch              |
   When I execute dnf with args "group install dnf-ci-testgroup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install-group | lame-0:3.100-4.fc29.x86_64                |
        | install-dep   | lame-libs-0:3.100-4.fc29.x86_64           |
        | group-install | DNF-CI-Testgroup                          |
        # filesystem package should be kept because they are user installed
   When I execute dnf with args "group remove dnf-ci-testgroup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | remove        | lame-0:3.100-4.fc29.x86_64                |
        | remove-unused | lame-libs-0:3.100-4.fc29.x86_64           |
        | group-remove  | DNF-CI-Testgroup                          |
        | unchanged     | filesystem-0:3.9-2.fc29.x86_64            |
        | unchanged     | setup-0:2.12.1-1.fc29.noarch              |


# TODO(nsella) "shell" command not implemented
# https://github.com/rpm-software-management/dnf5/issues/153
@xfail
@bz1809600
Scenario: Group remove does not traceback when reason change
  Given I use repository "dnf-ci-thirdparty"
    And I use repository "dnf-ci-fedora"
   When I execute dnf with args "group install DNF-CI-Testgroup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install-group | filesystem-0:3.9-2.fc29.x86_64            |
        | install-group | lame-0:3.100-4.fc29.x86_64                |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch              |
        | install-dep   | lame-libs-0:3.100-4.fc29.x86_64           |
        | group-install | DNF-CI-Testgroup                          |
   When I execute dnf with args "install basesystem"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | basesystem-0:11-6.fc29.noarch             |
        # filesystem package should be removed without Tracebacks in callbacks
  When I open dnf shell session
    And I execute in dnf shell "group remove DNF-CI-Testgroup"
    And I execute in dnf shell "remove filesystem"
    And I execute in dnf shell "run"
   Then Transaction is following
        | Action        | Package                                   |
        | remove        | lame-0:3.100-4.fc29.x86_64                |
        | remove        | filesystem-0:3.9-2.fc29.x86_64            |
        | remove-dep    | basesystem-0:11-6.fc29.noarch             |
        | remove-unused | setup-0:2.12.1-1.fc29.noarch              |
        | remove-unused | lame-libs-0:3.100-4.fc29.x86_64           |
        | group-remove  | DNF-CI-Testgroup                          |
    And stdout does not contain "Traceback .*"
   When I execute in dnf shell "exit"
   Then stdout contains "Leaving Shell"


@bz1706382
@dnf5daemon
Scenario: Group list
 Given I use repository "dnf-ci-thirdparty"
  When I execute dnf with args "group list"
  Then the exit code is 0
   And stderr is
       """
       <REPOSYNC>
       """
   And stdout is
       """
       ID                   Name                 Installed
       cqrlib-non-devel     CQRlib-non-devel            no
       dnf-ci-testgroup     DNF-CI-Testgroup            no
       superripper-and-deps SuperRipper-and-deps        no
       """


@dnf5daemon
@bz1706382
Scenario: Group list with arg
 Given I use repository "dnf-ci-thirdparty"
  When I execute dnf with args "group list dnf-ci-testgroup"
  Then the exit code is 0
   And stderr is
       """
       <REPOSYNC>
       """
   And stdout is
       """
       ID                   Name             Installed
       dnf-ci-testgroup     DNF-CI-Testgroup        no
       """


@bz1826198
Scenario: List an environment with empty name
  Given I use repository "comps-group"
  When I execute dnf with args "group list"
   Then the exit code is 0
   And stderr is
       """
       <REPOSYNC>
       """
   And stdout is
       """
       ID                   Name       Installed
       no-name-group                          no
       test-group           Test Group        no
       """


@bz1826198
Scenario: Install a group with empty name
  Given I use repository "comps-group"
  When I execute dnf with args "group install no-name-group"
   Then the exit code is 0
    # note the group is not listed in the transaction due to its name missing
    And Transaction is following
        | Action        | Package                           |
        | group-install | <name-unset>                      |
        | install-group | test-package-1.0-1.fc29.noarch    |


@bz1826198
Scenario: Install an environment with empty name
  Given I use repository "comps-group"
  When I execute dnf with args "group install no-name-env"
   Then the exit code is 0
    # note the env group is not listed in the transaction due to its name missing
    And Transaction is following
        | Action        | Package                           |
        | env-install   | <name-unset>                      |
        | group-install | Test Group                        |
        | install-group | test-package-1.0-1.fc29.noarch    |


@dnf5daemon
Scenario: List and info a group with missing packagelist
  Given I use repository "comps-group-merging"
   When I execute dnf with args "group list"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        ID                   Name       Installed
        test-group           Test Group        no
        """
   When I execute dnf with args "group info test-group"
   Then stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Id                   : test-group
        Name                 : Test Group
        Description          : Test Group description updated.
        Installed            : no
        Order                :
        Langonly             :
        Uservisible          : yes
        Repositories         : comps-group-merging
        """


Scenario: Install a group with empty packagelist
  Given I use repository "comps-group-merging"
   When I execute dnf with args "group install test-group"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package       |
        | group-install | Test Group    |


@not.with_os=rhel__ge__8
Scenario: Merge groups when one has empty packagelist
  Given I use repository "comps-group"
    And I use repository "comps-group-merging"
   When I execute dnf with args "group install test-group"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | group-install | Test Group                        |
        | install-group | test-package-1.0-1.fc29.noarch    |
   When I execute dnf with args "group info test-group"
   Then stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Id                   : test-group
        Name                 : Test Group
        Description          : Test Group description updated.
        Installed            : yes
        Order                : 2
        Langonly             :
        Uservisible          : yes
        Repositories         : @System
        Mandatory packages   : test-package
        """


# there is related issue
# https://github.com/rpm-software-management/dnf5/issues/724
Scenario: Merge environment with missing names containg a group with missing name
  Given I use repository "comps-group"
    And I use repository "comps-group-merging"
   When I execute dnf with args "environment info no-name-env"
   Then stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Id                   : no-name-env
        Name                 :
        Description          :
        Order                : 2
        Installed            : False
        Repositories         : comps-group, comps-group-merging
        Required groups      : no-name-group
                             : test-group
        """


@dnf5daemon
Scenario: Group info with a group that has missing name
  Given I use repository "comps-group"
   When I execute dnf with args "group info no-name-group"
   Then stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Id                   : no-name-group
        Name                 :
        Description          :
        Installed            : no
        Order                : 1
        Langonly             :
        Uservisible          : yes
        Repositories         : comps-group
        Mandatory packages   : test-package
        """


Scenario: Mark a group and an environment without name
  Given I use repository "comps-group"
    And I use repository "comps-group-merging"
   When I execute dnf with args "group install --no-packages no-name-group no-name-env"
   Then Transaction is following
        | Action        | Package                           |
        | env-install   | <name-unset>                      |
        | group-install | <name-unset>                      |
        | group-install | Test Group                        |


Scenario: Install an environment with a nonexistent group
  Given I use repository "comps-group"
  When I execute dnf with args "group install env-with-a-nonexistent-group"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | env-install   | Env with a nonexistent group      |
        | group-install | Test Group                        |
        | group-install | <name-unset>                      |
        | install-group | test-package-1.0-1.fc29.noarch    |
    And stderr contains lines
       """
       No match for group from environment: nonexistent-group
       """


Scenario: Install an environment using @^environment syntax
  Given I use repository "comps-group"
  When I execute dnf with args "install @^env-with-a-nonexistent-group"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | env-install   | Env with a nonexistent group      |
        | group-install | Test Group                        |
        | group-install | <name-unset>                      |
        | install-group | test-package-1.0-1.fc29.noarch    |
    And stderr contains lines
       """
       No match for group from environment: nonexistent-group
       """


@bz2066638
Scenario: Packages that are part of another installed group are not removed
  Given I use repository "comps-group"
        # install test-group and no-name-group, the test-package is part of both of them
   When I execute dnf with args "group install test-group"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | group-install | Test Group                        |
        | install-group | test-package-1.0-1.fc29.noarch    |
   When I execute dnf with args "group install no-name-group"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | group-install | <name-unset>                      |
        # after test-group removal, test-package is expected to stay installed
   When I execute dnf with args "group remove test-group"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package       |
        | group-remove  | Test Group    |


# destructive because it can create a new user on the system
@bz2030255
@destructive
Scenario: 'dnf group list -C' works for unprivileged user even when decompressed groups.xml is not present in the cache
 Given I use repository "dnf-ci-thirdparty"
    # unprivileged user will need access to enter installroot and read files there
   And I successfully execute "chmod go+rwx {context.dnf.installroot}"
    # unprivileged user will need tmp directory to create temporary decompressed groups.xml
   And I create directory "/{context.dnf.installroot}/var/tmp"
   And I successfully execute "chmod 777 {context.dnf.installroot}/var/tmp"
   And I successfully execute dnf with args "makecache"
  When I execute dnf with args "group list -C --skip-file-locks" as an unprivileged user
  Then the exit code is 0
  Then stderr does not contain "Permission denied: '{context.dnf.installroot}/var/cache/dnf/dnf-ci-thirdparty-[a-z0-9]{{16}}/repodata/gen'"
   And stderr is
       """
       <REPOSYNC>
       """
   And stdout is
       """
       ID                   Name                 Installed
       cqrlib-non-devel     CQRlib-non-devel            no
       dnf-ci-testgroup     DNF-CI-Testgroup            no
       superripper-and-deps SuperRipper-and-deps        no
       """


@dnf5daemon
@bz2173929
Scenario: dnf5 group list: empty output when run for the second time
 Given I use repository "advisories-and-groups"
  When I execute dnf with args "clean metadata"
   And I execute dnf with args "group list"
  Then the exit code is 0
   And stderr is
       """
       <REPOSYNC>
       """
   And stdout is
       """
       ID                   Name         Installed
       test-group-1         Test Group 1        no
       test-group-2         Test Group 2        no
       """
  When I execute dnf with args "group list"
  Then the exit code is 0
   And stderr is
       """
       <REPOSYNC>
       """
   And stdout is
       """
       ID                   Name         Installed
       test-group-1         Test Group 1        no
       test-group-2         Test Group 2        no
       """


# This typically covers situations where initial system state is created from
# dnf4 history db - dnf5 is not able to create installed groups *.xml files
# because dnf4 does not store their content anywhere.
Scenario: dnf can list installed groups even without their xml definitions present, group available
  Given I use repository "comps-group"
    And I successfully execute dnf with args "group install test-group"
    And I delete file "/usr/lib/sysimage/libdnf5/comps_groups/groups/test-group.xml"
   When I execute dnf with args "group info --installed"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Id                   : test-group
        Name                 : Test Group
        Description          : Test Group description.
        Installed            : yes
        Order                : 2
        Langonly             :
        Uservisible          : yes
        Repositories         : @System
        Mandatory packages   : test-package
        """


# In case the group is not available in repositories, minimal solvable is created
# from the system state.
Scenario: dnf can list installed groups even without their xml definitions present, group not available
  Given I use repository "comps-group"
    And I successfully execute dnf with args "group install test-group"
    And I delete file "/usr/lib/sysimage/libdnf5/comps_groups/groups/test-group.xml"
    And I drop repository "comps-group"
   When I execute dnf with args "group info --installed"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Id                   : test-group
        Name                 :
        Description          :
        Installed            : yes
        Order                :
        Langonly             :
        Uservisible          : yes
        Repositories         : @System
        Default packages     : test-package
        """


# https://github.com/rpm-software-management/dnf5/issues/917
Scenario: Remove group that is not installed
 Given I use repository "dnf-ci-thirdparty"
  When I execute dnf with args "group remove dnf-ci-testgroup"
  Then the exit code is 0
   And stderr is
       """
       <REPOSYNC>
       No groups to remove for argument: dnf-ci-testgroup
       """


@bz2263762
Scenario: Dnf works with comps group containing duplicate package entry
 Given I use repository "comps-group-duplicate-package"
  When I execute dnf with args "install @test-group"
  Then the exit code is 0
    And Transaction is following
        | Action        | Package       |
        | group-install | Test Group    |
