Feature: install with --from-vendor


Scenario: Install "vendorpkg" from vendor "First Vendor", dependencies from any vendor
  Given I use repository "dnf-ci-vendor-1"
    And I use repository "dnf-ci-vendor-1-updates"
    And I use repository "dnf-ci-vendor-2"
    And I use repository "dnf-ci-vendor-2-updates"
    And I use repository "dnf-ci-vendor-3"
    And I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "install --from-vendor='First\ Vendor' vendorpkg"
   Then the exit code is 0
    And Transaction is following
        | Action      | Package                  |
        | install     | vendorpkg-0:1.6-1.x86_64 |
        | install-dep | vendordep-0:1.8-1.x86_64 |
        | install-dep | vendordep2-0:1.2-1.x86_64 |
    And dnf5 transaction items for transaction "last" are
        | action  | package                    | reason          | repository                |
        | Install | vendorpkg-0:1.6-1.x86_64   | User            | dnf-ci-vendor-1-updates   |
        | Install | vendordep-0:1.8-1.x86_64   | Dependency      | dnf-ci-vendor-3-updates   |
        | Install | vendordep2-0:1.2-1.x86_64  | Dependency      | dnf-ci-vendor-3-updates   |


Scenario: Install "vendorpkg" and vendordep from vendor "First Vendor", not listed dependencies from any vendor
  Given I use repository "dnf-ci-vendor-1"
    And I use repository "dnf-ci-vendor-1-updates"
    And I use repository "dnf-ci-vendor-2"
    And I use repository "dnf-ci-vendor-2-updates"
    And I use repository "dnf-ci-vendor-3"
    And I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "install --from-vendor='First*' vendorpkg vendordep"
   Then the exit code is 0
    And Transaction is following
        | Action      | Package                  |
        | install     | vendorpkg-0:1.6-1.x86_64 |
        | install     | vendordep-0:1.6-1.x86_64 |
        | install-dep | vendordep2-0:1.2-1.x86_64 |
    And dnf5 transaction items for transaction "last" are
        | action  | package                    | reason          | repository                |
        | Install | vendorpkg-0:1.6-1.x86_64   | User            | dnf-ci-vendor-1-updates   |
        | Install | vendordep-0:1.6-1.x86_64   | User            | dnf-ci-vendor-1-updates   |
        | Install | vendordep2-0:1.2-1.x86_64  | Dependency      | dnf-ci-vendor-3-updates   |


Scenario: Install fails when package from requested vendor is not available
  Given I use repository "dnf-ci-vendor-1"
    And I use repository "dnf-ci-vendor-2"
   When I execute dnf with args "install --from-vendor='Third*' vendorpkg"
   Then the exit code is 1
