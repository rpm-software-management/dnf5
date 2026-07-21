Feature: Reinstall must keep the "reason" why a package was installed
  E.g. if package with dependency is installed, and the dependency is reinstalled, and the main package is then removed, the dependency is removed as well.


Scenario: Reinstall a dependency, and then remove the main package
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "install CQRlib-devel"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | CQRlib-devel-0:1.1.2-16.fc29.x86_64       |
        | install-dep   | CQRlib-0:1.1.2-16.fc29.x86_64             |
  Given I use repository "dnf-ci-fedora-updates-testing"
   When I execute dnf with args "reinstall CQRlib"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | reinstall     | CQRlib-0:1.1.2-16.fc29.x86_64             |
   When I execute dnf with args "remove CQRlib-devel"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | remove        | CQRlib-devel-0:1.1.2-16.fc29.x86_64       |
        | remove-unused | CQRlib-0:1.1.2-16.fc29.x86_64             |
