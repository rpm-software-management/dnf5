Feature: reinstall with --from-vendor


Background: Install some RPMs
  Given I use repository "dnf-ci-vendor-1"
   When I execute dnf with args "install vendorapp2 vendorpkg"
   Then the exit code is 0
    And Transaction is following
        | Action      | Package                   |
        | install     | vendorpkg-0:1.0-1.x86_64  |
        | install     | vendorapp2-0:2.0-1.x86_64 |
        | install-dep | vendordep-0:1.0-1.x86_64  |


Scenario: Reinstall vendorapp2 from "Second Vendor" (install new dep from any vendor)
  Given I use repository "dnf-ci-vendor-1-updates"
    And I use repository "dnf-ci-vendor-2"
    And I use repository "dnf-ci-vendor-2-updates"
    And I use repository "dnf-ci-vendor-3"
    And I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "reinstall --setopt=allow_vendor_change=1 --from-vendor='Second\ Vendor' vendorapp2"
   Then the exit code is 0
    And Transaction is following
        | Action       | Package                   |
        | reinstall    | vendorapp2-0:2.0-1.x86_64 |
        | upgrade      | vendordep-0:1.1-1.x86_64  |
        | install-weak | vendorweak-0:1.8-1.x86_64 |
    And dnf5 transaction items for transaction "last" are
        | action    | package                    | reason          | repository              |
        | Install   | vendorweak-0:1.8-1.x86_64  | Weak Dependency | dnf-ci-vendor-3-updates |
        | Reinstall | vendorapp2-0:2.0-1.x86_64  | User            | dnf-ci-vendor-2         |
        | Upgrade   | vendordep-0:1.1-1.x86_64   | Dependency      | dnf-ci-vendor-2         |
        | Replaced  | vendorapp2-0:2.0-1.x86_64  | User            | @System                 |
        | Replaced  | vendordep-0:1.0-1.x86_64   | Dependency      | @System                 |


Scenario: Reinstall vendorapp2 from "Second Vendor" fails (allow_vendor_change=0 prevents dependency upgrade)
  Given I use repository "dnf-ci-vendor-1-updates"
    And I use repository "dnf-ci-vendor-2"
    And I use repository "dnf-ci-vendor-2-updates"
    And I use repository "dnf-ci-vendor-3"
    And I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "reinstall --setopt=allow_vendor_change=0 --from-vendor='Second\ Vendor' vendorapp2"
   Then the exit code is 1
    And stderr contains "- package vendorapp2-2.0-1.x86_64 from dnf-ci-vendor-2 requires vendordep = 1.1, but none of the providers can be installed"
