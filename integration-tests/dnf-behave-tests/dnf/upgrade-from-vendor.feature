Feature: upgrade with --from-vendor


Background: Install some RPMs
  Given I use repository "dnf-ci-vendor-1"
   When I execute dnf with args "install vendorpkg vendorapp"
   Then the exit code is 0
    And Transaction is following
        | Action       | Package                   |
        | install      | vendorpkg-0:1.0-1.x86_64  |
        | install      | vendorapp-0:2.0-1.x86_64  |
        | install-dep  | vendordep-0:1.0-1.x86_64  |
        | install-weak | vendorweak-0:1.0-1.x86_64 |


Scenario: Upgrade vendorapp from "Second Vendor"
  Given I use repository "dnf-ci-vendor-1-updates"
    And I use repository "dnf-ci-vendor-2"
    And I use repository "dnf-ci-vendor-2-updates"
    And I use repository "dnf-ci-vendor-3"
    And I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "upgrade --setopt=allow_vendor_change=1 --from-vendor='Second\ Vendor' vendorapp"
   Then the exit code is 0
    And Transaction is following
        | Action  | Package                  |
        | upgrade | vendorapp-0:2.7-1.x86_64 |
    And dnf5 transaction items for transaction "last" are
        | action   | package                   | reason     | repository              |
        | Upgrade  | vendorapp-0:2.7-1.x86_64  | User       | dnf-ci-vendor-2-updates |
        | Replaced | vendorapp-0:2.0-1.x86_64  | User       | @System                 |


Scenario: Upgrade vendorapp from "Second Vendor" (--from-vendor has higher priority than allow_vendor_change=0)
  Given I use repository "dnf-ci-vendor-1-updates"
    And I use repository "dnf-ci-vendor-2"
    And I use repository "dnf-ci-vendor-2-updates"
    And I use repository "dnf-ci-vendor-3"
    And I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "upgrade --setopt=allow_vendor_change=0 --from-vendor='Second\ Vendor' vendorapp"
   Then the exit code is 0
    And Transaction is following
        | Action  | Package                  |
        | upgrade | vendorapp-0:2.7-1.x86_64 |
    And dnf5 transaction items for transaction "last" are
        | action   | package                   | reason     | repository              |
        | Upgrade  | vendorapp-0:2.7-1.x86_64  | User       | dnf-ci-vendor-2-updates |
        | Replaced | vendorapp-0:2.0-1.x86_64  | User       | @System                 |


Scenario: Upgrade vendorapp from any vendor (--from-vendor has higher priority than allow_vendor_change=0)
  Given I use repository "dnf-ci-vendor-1-updates"
    And I use repository "dnf-ci-vendor-2"
    And I use repository "dnf-ci-vendor-2-updates"
    And I use repository "dnf-ci-vendor-3"
    And I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "upgrade --setopt=allow_vendor_change=0 --from-vendor='*' vendorapp"
   Then the exit code is 0
    And Transaction is following
        | Action  | Package                  |
        | upgrade | vendorapp-0:2.8-1.x86_64 |
    And dnf5 transaction items for transaction "last" are
        | action   | package                   | reason     | repository              |
        | Upgrade  | vendorapp-0:2.8-1.x86_64  | User       | dnf-ci-vendor-3-updates |
        | Replaced | vendorapp-0:2.0-1.x86_64  | User       | @System                 |


Scenario: Upgrade vendorpkg from "Second Vendor" and upgrade dependencies and install new dependency from any repos
  Given I use repository "dnf-ci-vendor-1-updates"
    And I use repository "dnf-ci-vendor-2"
    And I use repository "dnf-ci-vendor-2-updates"
    And I use repository "dnf-ci-vendor-3"
    And I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "upgrade --setopt=allow_vendor_change=1 --from-vendor='Second\ Vendor' vendorpkg"
   Then the exit code is 0
    And Transaction is following
        | Action      | Package                   |
        | upgrade     | vendorpkg-0:1.10-1.x86_64 |
        | upgrade     | vendordep-0:1.8-1.x86_64  |
        | install-dep | vendordep2-0:1.1-1.x86_64 |
        | install-dep | vendordep3-0:1.2-1.x86_64 |
    And dnf5 transaction items for transaction "last" are
        | action   | package                    | reason          | repository              |
        | Install  | vendordep3-0:1.2-1.x86_64  | Dependency      | dnf-ci-vendor-3-updates |
        | Install  | vendordep2-0:1.1-1.x86_64  | Dependency      | dnf-ci-vendor-2-updates |
        | Upgrade  | vendorpkg-0:1.10-1.x86_64  | User            | dnf-ci-vendor-2-updates |
        | Upgrade  | vendordep-0:1.8-1.x86_64   | Dependency      | dnf-ci-vendor-3-updates |
        | Replaced | vendordep-0:1.0-1.x86_64   | Dependency      | @System                 |
        | Replaced | vendorpkg-0:1.0-1.x86_64   | User            | @System                 |


Scenario: Upgrade vendorpkg from "Second Vendor", don't change vendor for upgraded dependencies and install new dependency from any repos
  Given I use repository "dnf-ci-vendor-1-updates"
    And I use repository "dnf-ci-vendor-2"
    And I use repository "dnf-ci-vendor-2-updates"
    And I use repository "dnf-ci-vendor-3"
    And I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "upgrade --setopt=allow_vendor_change=0 --from-vendor='Second\ Vendor' vendorpkg"
   Then the exit code is 0
    And Transaction is following
        | Action      | Package                   |
        | upgrade     | vendorpkg-0:1.10-1.x86_64 |
        | upgrade     | vendordep-0:1.6-1.x86_64  |
        | install-dep | vendordep2-0:1.1-1.x86_64 |
        | install-dep | vendordep3-0:1.2-1.x86_64 |
    And dnf5 transaction items for transaction "last" are
        | action   | package                    | reason          | repository                |
        | Install  | vendordep3-0:1.2-1.x86_64  | Dependency      | dnf-ci-vendor-3-updates   |
        | Install  | vendordep2-0:1.1-1.x86_64  | Dependency      | dnf-ci-vendor-2-updates   |
        | Upgrade  | vendorpkg-0:1.10-1.x86_64  | User            | dnf-ci-vendor-2-updates   |
        | Upgrade  | vendordep-0:1.6-1.x86_64   | Dependency      | dnf-ci-vendor-1-updates   |
        | Replaced | vendordep-0:1.0-1.x86_64   | Dependency      | @System                   |
        | Replaced | vendorpkg-0:1.0-1.x86_64   | User            | @System                   |


Scenario: Use upgrades from "Second Vendor" and install new dependency from any repos (allow_vendor_change=1)
  Given I use repository "dnf-ci-vendor-1-updates"
    And I use repository "dnf-ci-vendor-2"
    And I use repository "dnf-ci-vendor-2-updates"
    And I use repository "dnf-ci-vendor-3"
    And I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "upgrade --no-best --setopt=allow_vendor_change=1 --from-vendor='Second\ Vendor' '*'"
   Then the exit code is 0
    And Transaction is following
        | Action      | Package                   |
        | upgrade     | vendorapp-0:2.7-1.x86_64  |
        | upgrade     | vendordep-0:1.1-1.x86_64  |
        | upgrade     | vendorpkg-0:1.7-1.x86_64  |
        | upgrade     | vendorweak-0:1.7-1.x86_64 |
        | install-dep | vendordep2-0:1.2-1.x86_64 |
        | broken      | vendorpkg-0:1.10-1.x86_64 |
        | conflict    | vendordep-0:1.2-1.x86_64  |
        | conflict    | vendordep-0:1.6-1.x86_64  |
        | conflict    | vendordep-0:1.8-1.x86_64  |
    And dnf5 transaction items for transaction "last" are
        | action          | package                    | reason          | repository              |
        | Install         | vendordep2-0:1.2-1.x86_64  | Dependency      | dnf-ci-vendor-3-updates |
        | Upgrade         | vendorapp-0:2.7-1.x86_64   | User            | dnf-ci-vendor-2-updates |
        | Upgrade         | vendordep-0:1.1-1.x86_64   | Dependency      | dnf-ci-vendor-2         |
        | Upgrade         | vendorpkg-0:1.7-1.x86_64   | User            | dnf-ci-vendor-2-updates |
        | Upgrade         | vendorweak-0:1.7-1.x86_64  | Weak Dependency | dnf-ci-vendor-2-updates |
        | Replaced        | vendorapp-0:2.0-1.x86_64   | User            | @System                 |
        | Replaced        | vendordep-0:1.0-1.x86_64   | Dependency      | @System                 |
        | Replaced        | vendorpkg-0:1.0-1.x86_64   | User            | @System                 |
        | Replaced        | vendorweak-0:1.0-1.x86_64  | Weak Dependency | @System                 |


Scenario: Use upgrades from "Second Vendor" and install new dependency from any repos (allow_vendor_change=0)
  Given I use repository "dnf-ci-vendor-1-updates"
    And I use repository "dnf-ci-vendor-2"
    And I use repository "dnf-ci-vendor-2-updates"
    And I use repository "dnf-ci-vendor-3"
    And I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "upgrade --no-best --setopt=allow_vendor_change=0 --from-vendor='Second\ Vendor' '*'"
   Then the exit code is 0
    And Transaction is following
        | Action      | Package                   |
        | upgrade     | vendorapp-0:2.7-1.x86_64  |
        | upgrade     | vendordep-0:1.1-1.x86_64  |
        | upgrade     | vendorpkg-0:1.7-1.x86_64  |
        | upgrade     | vendorweak-0:1.7-1.x86_64 |
        | install-dep | vendordep2-0:1.2-1.x86_64 |
        | broken      | vendorpkg-0:1.10-1.x86_64 |
        | conflict    | vendordep-0:1.2-1.x86_64  |
        | conflict    | vendordep-0:1.6-1.x86_64  |
        | conflict    | vendordep-0:1.8-1.x86_64  |
    And dnf5 transaction items for transaction "last" are
        | action          | package                    | reason          | repository              |
        | Install         | vendordep2-0:1.2-1.x86_64  | Dependency      | dnf-ci-vendor-3-updates |
        | Upgrade         | vendorapp-0:2.7-1.x86_64   | User            | dnf-ci-vendor-2-updates |
        | Upgrade         | vendordep-0:1.1-1.x86_64   | Dependency      | dnf-ci-vendor-2         |
        | Upgrade         | vendorpkg-0:1.7-1.x86_64   | User            | dnf-ci-vendor-2-updates |
        | Upgrade         | vendorweak-0:1.7-1.x86_64  | Weak Dependency | dnf-ci-vendor-2-updates |
        | Replaced        | vendorapp-0:2.0-1.x86_64   | User            | @System                 |
        | Replaced        | vendordep-0:1.0-1.x86_64   | Dependency      | @System                 |
        | Replaced        | vendorpkg-0:1.0-1.x86_64   | User            | @System                 |
        | Replaced        | vendorweak-0:1.0-1.x86_64  | Weak Dependency | @System                 |


Scenario: Upgrade fails when trying to upgrade from wrong vendor
  Given I use repository "dnf-ci-vendor-2-updates"
    And I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "upgrade --from-vendor='Second Vendor' vendorpkg"
   Then the exit code is 1
