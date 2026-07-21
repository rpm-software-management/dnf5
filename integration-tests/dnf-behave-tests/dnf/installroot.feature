Feature: Installroot test


@dnf5daemon
@force_installroot
Scenario: Install package from host repository into empty installroot
  # The following two steps generate repodata for the repository without configuring it
  Given I use repository "dnf-ci-install-remove"
  Given I drop repository "dnf-ci-install-remove"
    And I configure a new repository "outside-installroot" in "{context.dnf.tempdir}/repos.d" with
        | key     | value                                              |
        | baseurl | {context.scenario.repos_location}/dnf-ci-install-remove |
    When I execute dnf with args "--setopt=reposdir={context.dnf.tempdir}/repos.d install water_carbonated"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install       | water_carbonated-0:1.0-1.x86_64   |
   When I execute rpm on host with args "-q water_carbonated"
   Then the exit code is 1


@dnf5daemon
@force_installroot
Scenario: Install package from installroot repository into installroot
  Given I use repository "dnf-ci-install-remove"
   When I execute dnf with args "install water_carbonated"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install       | water_carbonated-0:1.0-1.x86_64   |
   When I execute rpm on host with args "-q water_carbonated"
   Then the exit code is 1


@force_installroot
Scenario: Test metadata handling in installroot
  Given I use repository "dnf-ci-install-remove"
   When I execute dnf with args "install water_carbonated"
   Then the exit code is 0
   When I execute "rm -rf {context.dnf.installroot}/var/cache/dnf" in "{context.dnf.installroot}"
   Then the exit code is 0
   When I execute dnf with args "repoquery -C water_still"
   Then the exit code is 1
   When I execute dnf with args "makecache"
   Then the exit code is 0
   When I execute dnf with args "repoquery -C water_still"
   Then the exit code is 0


@dnf5daemon
@force_installroot
Scenario: Remove package from installroot
  Given I use repository "dnf-ci-install-remove"
   When I execute dnf with args "install water_carbonated tea"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install       | water_carbonated-0:1.0-1.x86_64   |
        | install       | tea-0:1.0-1.x86_64                |
        | install-dep   | water-0:1.0-1.x86_64              |
   When I execute dnf with args "remove water_carbonated"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | remove        | water_carbonated-0:1.0-1.x86_64   |


@force_installroot
Scenario: Repolist command in installroot and with a reposdir specified
  Given I use repository "dnf-ci-install-remove"
   When I execute dnf with args "repo list"
   Then the exit code is 0
    And stdout is
        """
        repo id               repo name
        dnf-ci-install-remove dnf-ci-install-remove test repository
        """
  Given I configure a new repository "testrepo" in "{context.dnf.tempdir}/repos.d" with
        | key     | value                                              |
        | baseurl | {context.scenario.repos_location}/dnf-ci-install-remove |
   When I execute dnf with args "--setopt=reposdir={context.dnf.tempdir}/repos.d repo list"
   Then the exit code is 0
    And stdout is
        """
        repo id  repo name
        testrepo testrepo test repository
        """


@dnf5daemon
@force_installroot
Scenario: Upgrade package in installroot
  Given I use repository "dnf-ci-install-remove"
   When I execute dnf with args "install sugar-1.0"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install       | sugar-0:1.0-1.x86_64              |
   When I execute dnf with args "upgrade"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | upgrade       | sugar-0:2.0-1.x86_64              |
   When I execute rpm on host with args "-q sugar"
   Then the exit code is 1


@xfail
# (emrakova) in fact, the requested info is not provided:
# stderr:
#   Failed to resolve the transaction:
#   No match for argument: sugar
#   No repositories were loaded from the installroot. To use the configuration and repositories of the host system, pass --use-host-config.
# issue: https://github.com/rpm-software-management/dnf5/issues/1325
@bz1658579
Scenario: Installroot directory is listed when there are no repos
   When I execute dnf with args "install sugar"
   Then the exit code is 1
    And stderr contains "Error: There are no enabled repositories in \"{context.dnf.installroot}/etc/yum.repos.d\", \"{context.dnf.installroot}/etc/yum/repos.d\", \"{context.dnf.installroot}/etc/distro.repos.d\""
