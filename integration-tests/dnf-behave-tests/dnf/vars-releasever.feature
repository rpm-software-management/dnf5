Feature: Subtitute releasever in baseurl

Background:
  Given I do not set releasever
    And I use repository "dnf-ci-fedora" with configuration
        | key     | value                                                         |
        | baseurl | file://{context.dnf.installroot}/temp-repos/base-f$releasever |


Scenario: Releasever is substituted in baseurl via a command line option
  Given I copy directory "{context.scenario.repos_location}/dnf-ci-fedora" to "/temp-repos/base-f0123"
    And I execute dnf with args "install setup --releasever=0123"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                       |
        | install       | setup-0:2.12.1-1.fc29.noarch  |


Scenario: Releasever is substituted in baseurl via a config file
  Given I copy directory "{context.scenario.repos_location}/dnf-ci-fedora" to "/temp-repos/base-f0123"
    And I create and substitute file "/etc/dnf/vars/releasever" with
        """
        0123
        """
    And I execute dnf with args "install setup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                       |
        | install       | setup-0:2.12.1-1.fc29.noarch  |


Scenario: Releasever is substituted in baseurl via a value detected from a fedora-release package
  Given I successfully execute rpm with args "-i --nodeps {context.dnf.fixturesdir}/repos/dnf-ci-fedora/noarch/fedora-release-29-1.noarch.rpm"
    And I copy directory "{context.scenario.repos_location}/dnf-ci-fedora" to "/temp-repos/base-f29"
    And I execute dnf with args "install setup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                       |
        | install       | setup-0:2.12.1-1.fc29.noarch  |


@bz1710761
Scenario: Releasever is substituted in baseurl via a value detected from 'system-release(releasever)' provide
  Given I successfully execute rpm with args "-i --nodeps {context.dnf.fixturesdir}/repos/dnf-ci-fedora-release/noarch/fedora-release-29-1.noarch.rpm"
    And I copy directory "{context.scenario.repos_location}/dnf-ci-fedora" to "/temp-repos/base-f123"
    And I execute dnf with args "install setup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                       |
        | install       | setup-0:2.12.1-1.fc29.noarch  |


Scenario: releasever_{major,minor} are substituted via values detected from 'system-release(releasever_{major,minor})' provides
  Given I successfully execute rpm with args "-i --nodeps {context.dnf.fixturesdir}/repos/dnf-ci-fedora-release/noarch/fedora-release-29-1.noarch.rpm"
    And I use repository "dnf-ci-fedora" with configuration
        | key     | value                                                                                             |
        | baseurl | file://{context.dnf.installroot}/temp-repos/base-f$releasever-$releasever_major-$releasever_minor |
    And I copy directory "{context.scenario.repos_location}/dnf-ci-fedora" to "/temp-repos/base-f123-45-67"
    And I execute dnf with args "install setup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                       |
        | install       | setup-0:2.12.1-1.fc29.noarch  |


@destructive
Scenario: Releasever is substituted in baseurl using vars loaded from the same location (host or installroot) as repos
  Given I do not set releasever
    And I copy directory "{context.scenario.repos_location}/dnf-ci-fedora" to "/temp-repos/base-f0123"
    And I use repository "dnf-ci-fedora" with configuration
        | key         | value |
        | baseurl     | file://{context.dnf.installroot}/temp-repos/base-f$releasever  |
    # defines releasever in installroot (prepended automatically)
    And I create and substitute file "/etc/dnf/vars/releasever" with
        """
        0123
        """
    And I execute "mv {context.dnf.installroot}/etc/yum.repos.d/dnf-ci-fedora.repo /etc/yum.repos.d"
    And I delete directory "/etc/yum.repos.d"
   When I execute dnf with args "install --use-host-config setup --disablerepo=* --enablerepo=dnf-ci-fedora"
    Then the exit code is 1
  Given I execute "mv {context.dnf.installroot}/etc/dnf/vars/releasever /etc/dnf/vars/releasever"
    And I delete directory "/etc/dnf/vars"
   When I execute dnf with args "install --use-host-config setup --disablerepo=* --enablerepo=dnf-ci-fedora"
    Then the exit code is 0
    And Transaction is following
        | Action        | Package                       |
        | install       | setup-0:2.12.1-1.fc29.noarch  |


Scenario: Releasever is substituted in baseurl via vars in custom location
  Given I do not set releasever
    And I copy directory "{context.scenario.repos_location}/dnf-ci-fedora" to "/temp-repos/base-f0123"
    And I use repository "dnf-ci-fedora" with configuration
        | key         | value |
        | baseurl     | file://{context.dnf.installroot}/temp-repos/base-f$releasever  |
    And I create and substitute file "/tmp/vars/releasever" with
        """
        0123
        """
  When I execute dnf with args "install setup"
    Then the exit code is 1
  When I execute dnf with args "install setup --setopt=varsdir={context.dnf.installroot}/tmp/vars"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                       |
        | install       | setup-0:2.12.1-1.fc29.noarch  |


Scenario: Releasever gets split into releasever_major and releasever_minor
  Given I copy directory "{context.scenario.repos_location}/dnf-ci-fedora" to "/temp-repos/base-epel-12-34"
    And I use repository "dnf-ci-fedora" with configuration
        | key         | value |
        | baseurl     | file://{context.dnf.installroot}/temp-repos/base-epel-$releasever_major-$releasever_minor |
    And I execute dnf with args "install setup --releasever=12.34"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                       |
        | install       | setup-0:2.12.1-1.fc29.noarch  |


Scenario: releasever_major, releasever_minor can be overridden by command-line options
  Given I copy directory "{context.scenario.repos_location}/dnf-ci-fedora" to "/temp-repos/base-epel-9.9-12-34"
    And I use repository "dnf-ci-fedora" with configuration
        | key         | value |
        | baseurl     | file://{context.dnf.installroot}/temp-repos/base-epel-$releasever-$releasever_major-$releasever_minor |
    And I execute dnf with args "install setup --releasever=9.9 --releasever-major=12 --releasever-minor=34"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                       |
        | install       | setup-0:2.12.1-1.fc29.noarch  |


Scenario: Overriden only releasever_major
  Given I copy directory "{context.scenario.repos_location}/dnf-ci-fedora" to "/temp-repos/base-epel-12-9"
    And I use repository "dnf-ci-fedora" with configuration
        | key         | value |
        | baseurl     | file://{context.dnf.installroot}/temp-repos/base-epel-$releasever_major-$releasever_minor |
    And I execute dnf with args "install setup --releasever=9.9 --releasever-major=12"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                       |
        | install       | setup-0:2.12.1-1.fc29.noarch  |
