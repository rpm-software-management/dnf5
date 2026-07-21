Feature: Remove RPMs with --exclude


Scenario: Remove RPMs while excluding another RPM
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install basesystem"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | basesystem-0:11-6.fc29.noarch         |
        | install-dep   | filesystem-0:3.9-2.fc29.x86_64        |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch          |
   When I execute dnf with args "remove basesystem filesystem --exclude setup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | remove        | filesystem-0:3.9-2.fc29.x86_64        |
        | remove        | basesystem-0:11-6.fc29.noarch         |


Scenario: Remove RPM which is required by excluded RPM
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install filesystem"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | filesystem-0:3.9-2.fc29.x86_64        |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch          |
   When I execute dnf with args "remove setup --exclude filesystem"
   Then the exit code is 1
    And Transaction is empty
