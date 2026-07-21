@dnf5daemon
Feature: Install RPMs by pkgspec

Background: Use dnf-ci-fedora repository
  Given I use repository "dnf-ci-fedora"

@tier1
Scenario: Install an RPM by name
   When I execute dnf with args "install filesystem"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | filesystem-0:3.9-2.fc29.x86_64        |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch          |


@bz1734350
Scenario: Install packages by name from remote repodata with remote packages
Given I use repository "dnf-ci-fedora" as http
 When I execute dnf with args "install lame"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | install       | lame-0:3.100-4.fc29.x86_64                |
      | install-dep   | lame-libs-0:3.100-4.fc29.x86_64           |


Scenario: Install an RPM by name-version
   When I execute dnf with args "install filesystem-3.9"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | filesystem-0:3.9-2.fc29.x86_64        |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch          |


Scenario: Install an RPM by name-version-release
   When I execute dnf with args "install filesystem-3.9-2.fc29"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | filesystem-0:3.9-2.fc29.x86_64        |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch          |


Scenario: Install an RPM by name-version-release.arch
   When I execute dnf with args "install filesystem-3.9-2.fc29.x86_64"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | filesystem-0:3.9-2.fc29.x86_64        |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch          |


Scenario: Install an RPM by name-epoch:version-release.arch
   When I execute dnf with args "install filesystem-0:3.9-2.fc29.x86_64"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | filesystem-0:3.9-2.fc29.x86_64        |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch          |


Scenario: I can install an RPM by $pkgspec where $pkgspec is name.arch
  Given I use repository "dnf-ci-fedora-updates"
    And I execute dnf with args "install flac.x86_64"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | flac-0:1.3.3-3.fc29.x86_64            |


Scenario: I can install an RPM by $pkgspec where $pkgspec contains name with dashes
  Given I use repository "dnf-ci-fedora-updates-testing"
    And I execute dnf with args "install CQRlib-devel"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | CQRlib-devel-0:1.1.2-16.fc29.x86_64   |
        | install-dep   | CQRlib-0:1.1.2-16.fc29.x86_64         |


Scenario: I can install an RPM by $pkgspec where $pkgspec contains wildcards
  Given I use repository "dnf-ci-fedora-updates"
    And I execute dnf with args "install flac-*.3-2.fc29.x86_64"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | flac-0:1.3.3-2.fc29.x86_64            |
