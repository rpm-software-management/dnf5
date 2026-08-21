Feature: Install RPMs by provides


@dnf5daemon
Scenario: Install an RPM by provide that equals to e:v-r
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install 'filesystem = 0:3.9-2.fc29'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | filesystem-0:3.9-2.fc29.x86_64        |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch          |


@dnf5daemon
Scenario: Install an RPM by provide that is greater than e:vr
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install 'filesystem > 0:3.9-2'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | filesystem-0:3.9-2.fc29.x86_64        |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch          |


@dnf5daemon
Scenario: Install an RPM by provide that is greater than e:vr without space
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install 'filesystem >0:3.9-2'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | filesystem-0:3.9-2.fc29.x86_64        |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch          |


@dnf5daemon
Scenario: Install an RPM by provide that is greater or equal to e:vr
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install 'filesystem >= 0:3.9-2'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | filesystem-0:3.9-2.fc29.x86_64        |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch          |


@dnf5daemon
Scenario: Install an RPM by provide that is lower than e:vr
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "install 'glibc < 0:2.28-26.fc29'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | glibc-0:2.28-9.fc29.x86_64            |
        | install-dep   | glibc-common-0:2.28-9.fc29.x86_64     |
        | install-dep   | glibc-all-langpacks-0:2.28-9.fc29.x86_64      |
        | install-dep   | basesystem-0:11-6.fc29.noarch         |
        | install-dep   | filesystem-0:3.9-2.fc29.x86_64        |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch          |


@dnf5daemon
Scenario: Install an RPM by provide that is lower or equal to e:vr
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "install 'glibc <= 0:2.28-26.fc29'"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | glibc-0:2.28-26.fc29.x86_64           |
        | install-dep   | glibc-common-0:2.28-26.fc29.x86_64    |
        | install-dep   | glibc-all-langpacks-0:2.28-26.fc29.x86_64     |
        | install-dep   | basesystem-0:11-6.fc29.noarch         |
        | install-dep   | filesystem-0:3.9-2.fc29.x86_64        |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch          |


@dnf5daemon
Scenario: I can install an RPM by $provide where $provide is key
  Given I use repository "dnf-ci-fedora"
    And I execute dnf with args "install webclient"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | wget-0:1.19.5-5.fc29.x86_64               |


Scenario Outline: I can install an RPM by <provide type>
  Given I use repository "dnf-ci-fedora"
    And I execute dnf with args "install <provide> "
 Then the exit code is 0
  And Transaction is following
       | Action        | Package                                   |
       | install       | glibc-0:2.28-9.fc29.x86_64                |
       | install-dep   | setup-0:2.12.1-1.fc29.noarch              |
       | install-dep   | glibc-all-langpacks-0:2.28-9.fc29.x86_64  |
       | install-dep   | glibc-common-0:2.28-9.fc29.x86_64         |
       | install-dep   | filesystem-0:3.9-2.fc29.x86_64            |
       | install-dep   | basesystem-0:11-6.fc29.noarch             |

@dnf5daemon
Examples:
        | provide type                 | provide          |
        | file provide                 | /etc/ld.so.conf  |
        | file provide with wildcards  | /etc/ld*conf     |

Examples:
        | directory provide            | /var/db/         |

@dnf5daemon
Scenario: Install package using binary name
#  wget provides binary wget-bivary, but it is not explicitly in provides
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install wget-binary"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                  |
        | install       | wget-0:1.19.5-5.fc29.x86_64              |
