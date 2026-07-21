Feature: distro-sync with --from-vendor


Background: Install some RPMs
  Given I use repository "dnf-ci-vendor-1"
   When I execute dnf with args "install vendorapp2"
   Then the exit code is 0
    And Transaction is following
        | Action      | Package                   |
        | install     | vendorapp2-0:2.0-1.x86_64 |
        | install-dep | vendordep-0:1.0-1.x86_64  |
    And I use repository "dnf-ci-vendor-2"
    And I use repository "dnf-ci-vendor-3"
   When I execute dnf with args "install vendorapp"
   Then the exit code is 0
    And Transaction is following
        | Action       | Package                   |
        | install      | vendorapp-0:2.2-1.x86_64  |
        | install-weak | vendorweak-0:1.2-1.x86_64 |


Scenario: Distrosync specific packages from "Second Vendor" (allow_vendor_change=1)
  Given I use repository "dnf-ci-vendor-1-updates"
    And I use repository "dnf-ci-vendor-2-updates"
    And I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "distro-sync --setopt=allow_vendor_change=1 --from-vendor='Second\ Vendor' vendorapp vendorapp2"
   Then the exit code is 0
    And Transaction is following
        | Action    | Package                   |
        | reinstall | vendorapp2-0:2.0-1.x86_64 |
        | upgrade   | vendorapp-0:2.7-1.x86_64  |
        | upgrade   | vendordep-0:1.1-1.x86_64  |
    And dnf5 transaction items for transaction "last" are
        | action    | package                   | reason     | repository              |
        | Reinstall | vendorapp2-0:2.0-1.x86_64 | User       | dnf-ci-vendor-2         |
        | Upgrade   | vendorapp-0:2.7-1.x86_64  | User       | dnf-ci-vendor-2-updates |
        | Upgrade   | vendordep-0:1.1-1.x86_64  | Dependency | dnf-ci-vendor-2         |
        | Replaced  | vendorapp-0:2.2-1.x86_64  | User       | @System                 |
        | Replaced  | vendorapp2-0:2.0-1.x86_64 | User       | @System                 |
        | Replaced  | vendordep-0:1.0-1.x86_64  | Dependency | @System                 |


Scenario: Distrosync specific packages from "Second Vendor" fails (allow_vendor_change=0)
  Given I use repository "dnf-ci-vendor-1-updates"
    And I use repository "dnf-ci-vendor-2-updates"
    And I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "distro-sync --setopt=allow_vendor_change=0 --from-vendor='Second\ Vendor' vendorapp vendorapp2"
   Then the exit code is 1
    And stderr contains "- package vendorapp2-2.0-1.x86_64 from dnf-ci-vendor-2 requires vendordep = 1.1, but none of the providers can be installed"


Scenario: Distrosync all packages using wildcard from "Second Vendor" (allow_vendor_change=1)
  Given I use repository "dnf-ci-vendor-1-updates"
    And I use repository "dnf-ci-vendor-2-updates"
    And I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "distro-sync --setopt=allow_vendor_change=1 --from-vendor='Second\ Vendor' '*'"
   Then the exit code is 0
    And Transaction is following
        | Action    | Package                   |
        | reinstall | vendorapp2-0:2.0-1.x86_64 |
        | upgrade   | vendorapp-0:2.7-1.x86_64  |
        | upgrade   | vendordep-0:1.1-1.x86_64  |
        | upgrade   | vendorweak-0:1.7-1.x86_64 |
    And dnf5 transaction items for transaction "last" are
        | action          | package                   | reason          | repository              |
        | Reinstall       | vendorapp2-0:2.0-1.x86_64 | User            | dnf-ci-vendor-2         |
        | Upgrade         | vendorapp-0:2.7-1.x86_64  | User            | dnf-ci-vendor-2-updates |
        | Upgrade         | vendordep-0:1.1-1.x86_64  | Dependency      | dnf-ci-vendor-2         |
        | Upgrade         | vendorweak-0:1.7-1.x86_64 | Weak Dependency | dnf-ci-vendor-2-updates |
        | Replaced        | vendorapp-0:2.2-1.x86_64  | User            | @System                 |
        | Replaced        | vendorapp2-0:2.0-1.x86_64 | User            | @System                 |
        | Replaced        | vendordep-0:1.0-1.x86_64  | Dependency      | @System                 |
        | Replaced        | vendorweak-0:1.2-1.x86_64 | Weak Dependency | @System                 |


Scenario: Distrosync all packages using wildcard from "Second Vendor" (allow_vendor_change=0)
  Given I use repository "dnf-ci-vendor-1-updates"
    And I use repository "dnf-ci-vendor-2-updates"
    And I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "distro-sync --setopt=allow_vendor_change=0 --from-vendor='Second\ Vendor' '*'"
   Then the exit code is 0
    And Transaction is following
        | Action    | Package                   |
        | reinstall | vendorapp2-0:2.0-1.x86_64 |
        | upgrade   | vendorapp-0:2.7-1.x86_64  |
        | upgrade   | vendordep-0:1.1-1.x86_64  |
        | upgrade   | vendorweak-0:1.7-1.x86_64 |
    And dnf5 transaction items for transaction "last" are
        | action          | package                   | reason          | repository              |
        | Reinstall       | vendorapp2-0:2.0-1.x86_64 | User            | dnf-ci-vendor-2         |
        | Upgrade         | vendorapp-0:2.7-1.x86_64  | User            | dnf-ci-vendor-2-updates |
        | Upgrade         | vendordep-0:1.1-1.x86_64  | Dependency      | dnf-ci-vendor-2         |
        | Upgrade         | vendorweak-0:1.7-1.x86_64 | Weak Dependency | dnf-ci-vendor-2-updates |
        | Replaced        | vendorapp-0:2.2-1.x86_64  | User            | @System                 |
        | Replaced        | vendorapp2-0:2.0-1.x86_64 | User            | @System                 |
        | Replaced        | vendordep-0:1.0-1.x86_64  | Dependency      | @System                 |
        | Replaced        | vendorweak-0:1.2-1.x86_64 | Weak Dependency | @System                 |
