Feature: DNF config files testing


Scenario: Test removal of dependency when clean_requirements_on_remove=false
  Given I use repository "dnf-ci-fedora"
    And I configure dnf with
        | key                          | value      |
        | exclude                      | filesystem |
        | clean_requirements_on_remove | False      |
   When I execute dnf with args "install --setopt=disable_excludes=main filesystem"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install       | filesystem-0:3.9-2.fc29.x86_64    |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch      |
   When I execute dnf with args "remove --setopt=disable_excludes=* filesystem"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | remove        | filesystem-0:3.9-2.fc29.x86_64    |


Scenario: Test with dnf.conf in installroot (dnf.conf is taken from installroot)
  Given I use repository "dnf-ci-fedora"
    And I configure dnf with
        | key                          | value      |
        | exclude                      | filesystem |
   When I execute dnf with args "install filesystem"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        Argument 'filesystem' matches only excluded packages.
        """


Scenario: Test with dnf.conf in installroot and --config (dnf.conf is taken from --config)
  Given I use repository "dnf-ci-fedora"
    And I configure dnf with
        | key     | value      |
        | exclude | filesystem |
    And I create file "/test/dnf.conf" with
    """
    [main]
    exclude=dwm
    """
   When I execute dnf with args "--config {context.dnf.installroot}/test/dnf.conf install filesystem"
   Then the exit code is 0
   When I execute dnf with args "--config {context.dnf.installroot}/test/dnf.conf install dwm"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        Argument 'dwm' matches only excluded packages.
        """


Scenario: Reposdir option in dnf.conf file in installroot
  Given I configure dnf with
        | key      | value      |
        | reposdir | /testrepos |
    And I generate repodata for repository "dnf-ci-fedora"
    And I configure a new repository "testrepo" in "{context.dnf.installroot}/testrepos" with
        | key     | value                                                   |
        | baseurl | {context.scenario.repos_location}/dnf-ci-fedora |
   When I execute dnf with args "install filesystem"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install       | filesystem-0:3.9-2.fc29.x86_64    |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch      |


Scenario: Reposdir option in dnf.conf file with --config option in installroot
  Given I create file "/test/dnf.conf" with
    """
    [main]
    reposdir=/testrepos
    """
    And I generate repodata for repository "dnf-ci-fedora"
    And I configure a new repository "testrepo" in "{context.dnf.installroot}/testrepos" with
        | key     | value                                                   |
        | baseurl | {context.scenario.repos_location}/dnf-ci-fedora |
   When I execute dnf with args "--config {context.dnf.installroot}/test/dnf.conf install filesystem"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install       | filesystem-0:3.9-2.fc29.x86_64    |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch      |


Scenario: Reposdir option set by --setopt
  Given I configure a new repository "testrepo" in "{context.dnf.installroot}/testrepos" with
        | key     | value                                                   |
        | baseurl | {context.scenario.repos_location}/dnf-ci-fedora |
    And I generate repodata for repository "dnf-ci-fedora"
   # fail due to unavailable repository
   When I execute dnf with args "install filesystem"
   Then the exit code is 1
   # fail due to path in setopt is not affected by installroot
   When I execute dnf with args "install --setopt=reposdir=/testrepos filesystem"
   Then the exit code is 1
   When I execute dnf with args "install --setopt=reposdir={context.dnf.installroot}/testrepos filesystem"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install       | filesystem-0:3.9-2.fc29.x86_64    |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch      |


@bz1512457
Scenario: Test usage of not existing config file
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "--config {context.dnf.installroot}/non/existing/dnf.conf list"
   Then the exit code is 1
    And stderr contains "Configuration file.*not found"


Scenario: Test using the short alias "-c" instead of "--config" (we'll use the test for a non-existent config file)
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "-c {context.dnf.installroot}/non/existing/dnf.conf list"
   Then the exit code is 1
    And stderr contains "Configuration file.*not found"


@bz1722493
Scenario: Lines that contain only whitespaces do not spoil previous config options
  Given I create file "/test/dnf.conf" with
    # the "empty" line between gpgcheck and baseurl intentionally contains spaces
    """
    [main]
    gpgcheck=0

    [testingrepo]
    gpgcheck=1

    baseurl=http://some.url/
    """
   When I execute dnf with args "--config={context.dnf.installroot}/test/dnf.conf --dump-repo-config=testingrepo"
   Then stdout contains lines
   """
   gpgcheck = 1
   """


# dnf5 does not support remote config files
# https://github.com/rpm-software-management/dnf5/issues/1767
@xfail
@bz1721091
Scenario: Dnf can use config file from remote location
  Given I create directory "/remotedir"
    And I create file "/remotedir/remote.conf" with
    """
    [repo-from-remote-config]
    baseurl=http://some.url/
    """
    And I set up a http server for directory "/remotedir"
    When I execute dnf with args "--config http://localhost:{context.dnf.ports[/remotedir]}/remote.conf repo list repo-from-remote-config"
   Then the exit code is 0
    And stdout is
    """
    repo id                             repo name                            status
    repo-from-remote-config             repo-from-remote-config              enabled
    """


@bz1721091
Scenario: Dnf prints reasonable error when remote config file is not downloadable
  Given I create directory "/remotedir"
    And I set up a http server for directory "/remotedir"
   # 404 not found
   When I execute dnf with args "--config http://localhost:{context.dnf.ports[/remotedir]}/does-not-exist.conf repo list repo-from-remote-config"
   Then the exit code is 1
   And stderr contains "Configuration file.*not found"
   # unsupported protocol
   When I execute dnf with args "--config xxxx://localhost:{context.dnf.ports[/remotedir]}/does-not-exist.conf repo list repo-from-remote-config"
   Then the exit code is 1
   And stderr contains "Configuration file.*not found"
   # host unknown
   When I execute dnf with args "--config http://the_host:{context.dnf.ports[/remotedir]}/does-not-exist.conf repo list repo-from-remote-config"
   Then the exit code is 1
   And stderr contains "Configuration file.*not found"


@no_installroot
Scenario: Create dnf.conf file and test if host is using /etc/dnf/dnf.conf
  Given I use repository "simple-base"
    And I create file "/etc/dnf/dnf.conf" with
    """
    [main]
    exclude=vagare
    """
   When I execute dnf with args "install vagare"
   Then the exit code is 1
    # rhsm plugin is polluting stderr with messages, we cannot use "stderr is" step
    # librhsm-WARNING **: 12:59:28.478: Found 0 entitlement certificates
    # librhsm-WARNING **: 12:59:28.478: Found 0 product certificates
    And stderr contains "Argument 'vagare' matches only excluded packages."


@no_installroot
Scenario: Create dnf.conf file and test if host is taking option --config /test/dnf.conf file
  Given I use repository "simple-base"
    And I create file "/etc/dnf/dnf.conf" with
    """
    [main]
    exclude=vagare
    """
    And I create file "/test/dnf.conf" with
    """
    [main]
    exclude=dedalo
    """
   When I execute dnf with args "--config /test/dnf.conf install vagare"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | vagare-1.0-1.fc29.x86_64              |
        | install-dep   | labirinto-1.0-1.fc29.x86_64           |
   When I execute dnf with args "--config /test/dnf.conf install dedalo"
   Then the exit code is 1
    # rhsm plugin is polluting stderr with messages, we cannot use "stderr is" step
    # librhsm-WARNING **: 12:59:28.478: Found 0 entitlement certificates
    # librhsm-WARNING **: 12:59:28.478: Found 0 product certificates
    And stderr contains "Argument 'dedalo' matches only excluded packages."


@destructive
Scenario: dnf.conf is not taken from host even even if the file in installroot does not exist
  Given I use repository "simple-base"
    # create host config file
    And I create and substitute file "//etc/dnf/dnf.conf" with
    """
    [main]
    exclude=vagare
    reposdir={context.dnf.installroot}/etc/yum.repos.d
    """
    # ensure there is no dnf.conf in the installroot
    And I delete file "/etc/dnf/dnf.conf"
   When I execute dnf with args "install vagare"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | vagare-1.0-1.fc29.x86_64              |
        | install-dep   | labirinto-1.0-1.fc29.x86_64           |


@destructive
Scenario: dnf.conf is taken from host if --use-host-config is used
  Given I use repository "simple-base"
    # create host config file
    And I create and substitute file "//etc/dnf/dnf.conf" with
    """
    [main]
    exclude=vagare
    reposdir={context.dnf.installroot}/etc/yum.repos.d
    """
    # ensure there is no dnf.conf in the installroot
    And I delete file "/etc/dnf/dnf.conf"
   When I execute dnf with args "install vagare --use-host-config"
   Then the exit code is 1
    And stderr is
    """
    <REPOSYNC>
    Failed to resolve the transaction:
    Argument 'vagare' matches only excluded packages.
    """


@no_installroot
Scenario: Reposdir option in dnf.conf file in host
  Given I configure dnf with
        | key      | value      |
        | reposdir | /testrepos |
    And I generate repodata for repository "simple-base"
    And I configure a new repository "testrepo" in "/testrepos" with
        | key     | value                                                   |
        | baseurl | {context.scenario.repos_location}/simple-base           |
   When I execute dnf with args "install vagare"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install       | vagare-1.0-1.fc29.x86_64          |
        | install-dep   | labirinto-1.0-1.fc29.x86_64       |
