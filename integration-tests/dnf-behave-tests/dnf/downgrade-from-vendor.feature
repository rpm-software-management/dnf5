Feature: downgrade with --from-vendor


Background: Install some RPMs
  Given I use repository "dnf-ci-vendor-3"
    And I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "install vendorpkg vendorapp"
   Then the exit code is 0
    And Transaction is following
        | Action       | Package                   |
        | install      | vendorapp-0:2.8-1.x86_64  |
        | install      | vendorpkg-0:1.12-1.x86_64 |
        | install-dep  | vendordep-0:1.8-1.x86_64  |
        | install-dep  | vendordep2-0:1.2-1.x86_64 |
        | install-weak | vendorweak-0:1.8-1.x86_64 |


Scenario: Downgrade vendorapp from "Second Vendor"
  Given I use repository "dnf-ci-vendor-1"
    And I use repository "dnf-ci-vendor-1-updates"
    And I use repository "dnf-ci-vendor-2"
    And I use repository "dnf-ci-vendor-2-updates"
   When I execute dnf with args "downgrade --setopt=allow_vendor_change=1 --from-vendor='Second\ Vendor' vendorapp"
   Then the exit code is 0
    And Transaction is following
        | Action    | Package                  |
        | downgrade | vendorapp-0:2.7-1.x86_64 |
    And dnf5 transaction items for transaction "last" are
        | action    | package                   | reason     | repository              |
        | Downgrade | vendorapp-0:2.7-1.x86_64  | User       | dnf-ci-vendor-2-updates |
        | Replaced  | vendorapp-0:2.8-1.x86_64  | User       | @System                 |


Scenario: Downgrade vendorapp from "Second Vendor" (--from-vendor has higher priority than allow_vendor_change=0)
  Given I use repository "dnf-ci-vendor-1"
    And I use repository "dnf-ci-vendor-1-updates"
    And I use repository "dnf-ci-vendor-2"
    And I use repository "dnf-ci-vendor-2-updates"
   When I execute dnf with args "downgrade --setopt=allow_vendor_change=0 --from-vendor='Second\ Vendor' vendorapp"
   Then the exit code is 0
    And Transaction is following
        | Action    | Package                  |
        | downgrade | vendorapp-0:2.7-1.x86_64 |
    And dnf5 transaction items for transaction "last" are
        | action    | package                   | reason     | repository              |
        | Downgrade | vendorapp-0:2.7-1.x86_64  | User       | dnf-ci-vendor-2-updates |
        | Replaced  | vendorapp-0:2.8-1.x86_64  | User       | @System                 |


Scenario: Downgrade vendorpkg from "Second Vendor" and downgrade dependencies and install new dependency from any repos
  Given I use repository "dnf-ci-vendor-1"
    And I use repository "dnf-ci-vendor-1-updates"
    And I use repository "dnf-ci-vendor-2"
    And I use repository "dnf-ci-vendor-2-updates"
   When I execute dnf with args "downgrade --setopt=allow_vendor_change=1 --from-vendor='Second\ Vendor' vendorpkg"
   Then the exit code is 0
    And Transaction is following
        | Action      | Package                   |
        | downgrade   | vendorpkg-0:1.10-1.x86_64 |
        | downgrade   | vendordep2-0:1.1-1.x86_64 |
        | install-dep | vendordep3-0:1.2-1.x86_64 |
    And dnf5 transaction items for transaction "last" are
        | action    | package                    | reason     | repository              |
        | Install   | vendordep3-0:1.2-1.x86_64  | Dependency | dnf-ci-vendor-3-updates |
        | Downgrade | vendorpkg-0:1.10-1.x86_64  | User       | dnf-ci-vendor-2-updates |
        | Downgrade | vendordep2-0:1.1-1.x86_64  | Dependency | dnf-ci-vendor-2-updates |
        | Replaced  | vendordep2-0:1.2-1.x86_64  | Dependency | @System                 |
        | Replaced  | vendorpkg-0:1.12-1.x86_64  | User       | @System                 |


Scenario: Downgrade vendorpkg from "Second Vendor", don't change vendor for downgraded dependencies and install new dependency from any repos
  Given I use repository "dnf-ci-vendor-1"
    And I use repository "dnf-ci-vendor-1-updates"
    And I use repository "dnf-ci-vendor-2"
    And I use repository "dnf-ci-vendor-2-updates"
   When I execute dnf with args "downgrade --no-best --setopt=allow_vendor_change=0 --from-vendor='Second\ Vendor' vendorpkg"
   Then the exit code is 0
    And Transaction is following
        | Action    | Package                   |
        | downgrade | vendorpkg-0:1.7-1.x86_64  |
        | conflict  | vendordep2-0:1.0-1.x86_64 |
        | conflict  | vendordep2-0:1.1-1.x86_64 |
        | broken    | vendorpkg-0:1.10-1.x86_64 |
    And dnf5 transaction items for transaction "last" are
        | action    | package                   | reason     | repository              |
        | Downgrade | vendorpkg-0:1.7-1.x86_64  | User       | dnf-ci-vendor-2-updates |
        | Replaced  | vendorpkg-0:1.12-1.x86_64 | User       | @System                 |


Scenario: Use downgrades from "Second Vendor" and install new dependency from any repos (allow_vendor_change=1)
  Given I use repository "dnf-ci-vendor-1"
    And I use repository "dnf-ci-vendor-1-updates"
    And I use repository "dnf-ci-vendor-2"
    And I use repository "dnf-ci-vendor-2-updates"
   When I execute dnf with args "downgrade --no-best --setopt=allow_vendor_change=1 --from-vendor='Second\ Vendor' '*'"
   Then the exit code is 0
    And Transaction is following
        | Action    | Package                   |
        | downgrade | vendorapp-0:2.7-1.x86_64  |
        | downgrade | vendordep-0:1.1-1.x86_64  |
        | downgrade | vendordep2-0:1.1-1.x86_64 |
        | downgrade | vendorpkg-0:1.7-1.x86_64  |
        | downgrade | vendorweak-0:1.7-1.x86_64 |
        | broken    | vendorpkg-0:1.10-1.x86_64 |
        | conflict  | vendordep-0:1.2-1.x86_64  |
        | conflict  | vendordep-0:1.6-1.x86_64  |
    And dnf5 transaction items for transaction "last" are
        | action          | package                    | reason          | repository              |
        | Downgrade       | vendordep-0:1.1-1.x86_64   | Dependency      | dnf-ci-vendor-2         |
        | Downgrade       | vendordep2-0:1.1-1.x86_64  | Dependency      | dnf-ci-vendor-2-updates |
        | Downgrade       | vendorapp-0:2.7-1.x86_64   | User            | dnf-ci-vendor-2-updates |
        | Downgrade       | vendorpkg-0:1.7-1.x86_64   | User            | dnf-ci-vendor-2-updates |
        | Downgrade       | vendorweak-0:1.7-1.x86_64  | Weak Dependency | dnf-ci-vendor-2-updates |
        | Replaced        | vendorapp-0:2.8-1.x86_64   | User            | @System                 |
        | Replaced        | vendordep-0:1.8-1.x86_64   | Dependency      | @System                 |
        | Replaced        | vendordep2-0:1.2-1.x86_64  | Dependency      | @System                 |
        | Replaced        | vendorpkg-0:1.12-1.x86_64  | User            | @System                 |
        | Replaced        | vendorweak-0:1.8-1.x86_64  | Weak Dependency | @System                 |


Scenario: Use downgrades from "Second Vendor" and downgrade new dependency from any repos (allow_vendor_change=0)
  Given I use repository "dnf-ci-vendor-1"
    And I use repository "dnf-ci-vendor-1-updates"
    And I use repository "dnf-ci-vendor-2"
    And I use repository "dnf-ci-vendor-2-updates"
   When I execute dnf with args "downgrade --no-best --setopt=allow_vendor_change=0 --from-vendor='Second\ Vendor' '*'"
   Then the exit code is 0
    And Transaction is following
        | Action    | Package                   |
        | downgrade | vendorapp-0:2.7-1.x86_64  |
        | downgrade | vendordep-0:1.1-1.x86_64  |
        | downgrade | vendordep2-0:1.1-1.x86_64 |
        | downgrade | vendorpkg-0:1.7-1.x86_64  |
        | downgrade | vendorweak-0:1.7-1.x86_64 |
        | broken    | vendorpkg-0:1.10-1.x86_64 |
        | conflict  | vendordep-0:1.2-1.x86_64  |
        | conflict  | vendordep-0:1.6-1.x86_64  |
    And dnf5 transaction items for transaction "last" are
        | action          | package                    | reason          | repository              |
        | Downgrade       | vendordep-0:1.1-1.x86_64   | Dependency      | dnf-ci-vendor-2         |
        | Downgrade       | vendordep2-0:1.1-1.x86_64  | Dependency      | dnf-ci-vendor-2-updates |
        | Downgrade       | vendorapp-0:2.7-1.x86_64   | User            | dnf-ci-vendor-2-updates |
        | Downgrade       | vendorpkg-0:1.7-1.x86_64   | User            | dnf-ci-vendor-2-updates |
        | Downgrade       | vendorweak-0:1.7-1.x86_64  | Weak Dependency | dnf-ci-vendor-2-updates |
        | Replaced        | vendorapp-0:2.8-1.x86_64   | User            | @System                 |
        | Replaced        | vendordep-0:1.8-1.x86_64   | Dependency      | @System                 |
        | Replaced        | vendordep2-0:1.2-1.x86_64  | Dependency      | @System                 |
        | Replaced        | vendorpkg-0:1.12-1.x86_64  | User            | @System                 |
        | Replaced        | vendorweak-0:1.8-1.x86_64  | Weak Dependency | @System                 |

Scenario: Downgrade fails when trying to downgrade from nonexist vendor
  Given I use repository "dnf-ci-vendor-1"
    And I use repository "dnf-ci-vendor-2-updates"
   When I execute dnf with args "downgrade --from-vendor='Nonexist\ Vendor' vendorpkg"
   Then the exit code is 1
