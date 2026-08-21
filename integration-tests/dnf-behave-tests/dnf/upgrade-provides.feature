Feature: Upgrade RPMs by provides


Background: Install glibc
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install glibc"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | glibc-0:2.28-9.fc29.x86_64                |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch              |
        | install-dep   | filesystem-0:3.9-2.fc29.x86_64            |
        | install-dep   | basesystem-0:11-6.fc29.noarch             |
        | install-dep   | glibc-common-0:2.28-9.fc29.x86_64         |
        | install-dep   | glibc-all-langpacks-0:2.28-9.fc29.x86_64  |


@dnf5daemon
Scenario Outline: Upgrade an RPM by provide <operator> e:v-r
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "upgrade 'glibc <operator> <e:v-r>'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | glibc-0:2.28-26.fc29.x86_64               |
        | upgrade       | glibc-common-0:2.28-26.fc29.x86_64        |
        | upgrade       | glibc-all-langpacks-0:2.28-26.fc29.x86_64 |

@tier1
Examples:
  | operator      | e:v-r                |
  | =             | 0:2.28-26.fc29       |
  | >             | 0:2.28-9.fc29        |
  | >=            | 0:2.28-26.fc29       |
  | <             | 0:2.28-27.fc29       |
  | <=            | 0:2.28-26.fc29       |


@dnf5daemon
Scenario: Upgrade an RPM by provide
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "upgrade 'libm.so.6()(64bit)'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | glibc-0:2.28-26.fc29.x86_64               |
        | upgrade       | glibc-common-0:2.28-26.fc29.x86_64        |
        | upgrade       | glibc-all-langpacks-0:2.28-26.fc29.x86_64 |


@dnf5daemon
Scenario: Upgrade an RPM by file provide
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "upgrade /etc/ld.so.conf"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | glibc-0:2.28-26.fc29.x86_64               |
        | upgrade       | glibc-common-0:2.28-26.fc29.x86_64        |
        | upgrade       | glibc-all-langpacks-0:2.28-26.fc29.x86_64 |


Scenario: Upgrade an RPM by file provide that is directory
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "upgrade /var/db"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | glibc-0:2.28-26.fc29.x86_64               |
        | upgrade       | glibc-common-0:2.28-26.fc29.x86_64        |
        | upgrade       | glibc-all-langpacks-0:2.28-26.fc29.x86_64 |


@dnf5daemon
Scenario: Upgrade an RPM by file provide containing wildcards
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "upgrade /etc/ld*.conf"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | glibc-0:2.28-26.fc29.x86_64               |
        | upgrade       | glibc-common-0:2.28-26.fc29.x86_64        |
        | upgrade       | glibc-all-langpacks-0:2.28-26.fc29.x86_64 |
