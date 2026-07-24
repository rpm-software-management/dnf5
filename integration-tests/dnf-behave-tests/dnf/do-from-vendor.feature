Feature: do with --from-vendor


Background: Install some RPMs
  Given I use repository "dnf-ci-vendor-1"
    And I use repository "dnf-ci-vendor-2"
    And I use repository "dnf-ci-vendor-3"
   When I execute dnf with args "do --action=install --from-vendor="First\ Vendor" vendorapp2 --from-vendor="Third\ Vendor" vendorapp"
   Then the exit code is 0
    And Transaction is following
        | Action       | Package                   |
        | install      | vendorapp-0:2.2-1.x86_64  |
        | install      | vendorapp2-0:2.0-1.x86_64 |
        | install-dep  | vendordep-0:1.0-1.x86_64  |
        | install-weak | vendorweak-0:1.2-1.x86_64 |
    And dnf5 transaction items for transaction "last" are
        | action    | package                    | reason          | repository            |
        | Install   | vendorapp2-0:2.0-1.x86_64  | User            | dnf-ci-vendor-1       |
        | Install   | vendorapp-0:2.2-1.x86_64   | User            | dnf-ci-vendor-3       |
        | Install   | vendordep-0:1.0-1.x86_64   | Dependency      | dnf-ci-vendor-1       |
        | Install   | vendorweak-0:1.2-1.x86_64  | Weak Dependency | dnf-ci-vendor-3       |


Scenario: Reinstall vendorapp2 from "Second Vendor" and downgrade vendorapp from "First Vendor" (allow_vendor_change=1)
  Given I use repository "dnf-ci-vendor-1-updates"
    And I use repository "dnf-ci-vendor-2-updates"
    And I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "do --action=reinstall --setopt=allow_vendor_change=1 --from-vendor='Second\ Vendor' vendorapp2 --action=downgrade --from-vendor='First\ Vendor' vendorapp"
   Then the exit code is 0
    And Transaction is following
        | Action    | Package                   |
        | reinstall | vendorapp2-0:2.0-1.x86_64 |
        | upgrade   | vendordep-0:1.1-1.x86_64  |
        | downgrade | vendorapp-0:2.0-1.x86_64  |
    And dnf5 transaction items for transaction "last" are
        | action    | package                    | reason          | repository            |
        | Reinstall | vendorapp2-0:2.0-1.x86_64  | User            | dnf-ci-vendor-2       |
        | Upgrade   | vendordep-0:1.1-1.x86_64   | Dependency      | dnf-ci-vendor-2       |
        | Downgrade | vendorapp-0:2.0-1.x86_64   | User            | dnf-ci-vendor-1       |
        | Replaced  | vendorapp-0:2.2-1.x86_64   | User            | @System               |
        | Replaced  | vendorapp2-0:2.0-1.x86_64  | User            | @System               |
        | Replaced  | vendordep-0:1.0-1.x86_64   | Dependency      | @System               |


Scenario: Reinstall vendorapp2 from "Second Vendor" and downgrade vendorapp from "First Vendor" fails (allow_vendor_change=0)
  Given I use repository "dnf-ci-vendor-1-updates"
    And I use repository "dnf-ci-vendor-2-updates"
    And I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "do --action=reinstall --setopt=allow_vendor_change=0 --from-vendor='Second\ Vendor' vendorapp2 --action=downgrade --from-vendor='First\ Vendor' vendorapp"
   Then the exit code is 1
    And stderr contains "- package vendorapp2-2.0-1.x86_64 from dnf-ci-vendor-2 requires vendordep = 1.1, but none of the providers can be installed"


Scenario: Reinstall vendorapp2 from "Second Vendor" and downgrade vendorapp from "First Vendor" with --skip-broken (allow_vendor_change=0)
  Given I use repository "dnf-ci-vendor-1-updates"
    And I use repository "dnf-ci-vendor-2-updates"
    And I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "do --skip-broken --action=reinstall --setopt=allow_vendor_change=0 --from-vendor='Second\ Vendor' vendorapp2 --action=downgrade --from-vendor='First\ Vendor' vendorapp"
   Then the exit code is 0
    And Transaction is following
        | Action    | Package                   |
        | downgrade | vendorapp-0:2.0-1.x86_64  |
        | conflict  | vendordep-0:1.1-1.x86_64  |
        | conflict  | vendordep-0:1.6-1.x86_64  |
        | broken    | vendorapp2-0:2.0-1.x86_64 |
    And dnf5 transaction items for transaction "last" are
        | action    | package                    | reason          | repository            |
        | Downgrade | vendorapp-0:2.0-1.x86_64   | User            | dnf-ci-vendor-1       |
        | Replaced  | vendorapp-0:2.2-1.x86_64   | User            | @System               |
