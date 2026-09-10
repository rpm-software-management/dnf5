Feature: DNF allow_vendor_change option with policies version "1.2"


Background:
  Given I use repository "dnf-ci-vendor-1"
    And I create and substitute file "/etc/dnf/dnf.conf" with
    """
    [main]
    allow_vendor_change=False
    """
    And I successfully execute dnf with args "install vendor vendorpkg"
    Then the exit code is 0
  Given I successfully execute rpm with args "--queryformat='%{{name}}-%{{version}}-%{{release}} : %{{vendor}}\n' -q -a"
   Then the exit code is 0
    And stdout contains lines
    """
    vendor-1.0-1 : First Vendor
    vendorpkg-1.0-1 : First Vendor
    vendordep-1.0-1 : First Vendor
    """


Scenario Outline: Upgrade with vendor policy - <comparator>
  Given I create file "/etc/dnf/vendors.d/test-policy.conf" with
    """
    version = '1.2'

    [[incoming_vendors]]
    vendor = '<vendor>'
    comparator = '<comparator>'
    """
  Given I use repository "dnf-ci-vendor-1-updates"
  Given I use repository "dnf-ci-vendor-2-updates"
  Given I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "upgrade '*'"
   Then the exit code is 0
  Given I successfully execute rpm with args "--queryformat='%{{name}}-%{{version}}-%{{release}} : %{{vendor}}\n' -q -i vendor"
   Then the exit code is 0
    And stdout contains lines
    """
    <stdout_line>
    """

Examples:
    | vendor    | comparator      | stdout_line                   |
    | Third     | NOT_STARTSWITH  | vendor-1.2-1 : Second Vendor  |
    | third     | NOT_STARTSWITH  | vendor-1.3-1 : Third Vendor   |
    | third     | NOT_ISTARTSWITH | vendor-1.2-1 : Second Vendor  |
    | five      | NOT_ISTARTSWITH | vendor-1.3-1 : Third Vendor   |
    | rd Vendor | NOT_ENDSWITH    | vendor-1.2-1 : Second Vendor  |
    | rd vendor | NOT_ENDSWITH    | vendor-1.3-1 : Third Vendor   |
    | rd vendor | NOT_IENDSWITH   | vendor-1.2-1 : Second Vendor  |
    | five      | NOT_IENDSWITH   | vendor-1.3-1 : Third Vendor   |
    | ^T.ird.*  | NOT_REGEX       | vendor-1.2-1 : Second Vendor  |
    | ^t.ird.*  | NOT_REGEX       | vendor-1.3-1 : Third Vendor   |
    | ^t.ird.*  | NOT_IREGEX      | vendor-1.2-1 : Second Vendor  |
    | ^p.ird.*  | NOT_IREGEX      | vendor-1.3-1 : Third Vendor   |


Scenario Outline: Upgrade with vendor policy - <comparator> (--add-vendor-policy)
  Given I use repository "dnf-ci-vendor-1-updates"
  Given I use repository "dnf-ci-vendor-2-updates"
  Given I use repository "dnf-ci-vendor-3-updates"
   When I execute dnf with args "--add-vendor-policy=in:<comparator>\"<vendor>\" upgrade '*'"
   Then the exit code is 0
  Given I successfully execute rpm with args "--queryformat='%{{name}}-%{{version}}-%{{release}} : %{{vendor}}\n' -q -i vendor"
   Then the exit code is 0
    And stdout contains lines
    """
    <stdout_line>
    """

Examples:
    | vendor     | comparator | stdout_line                   |
    | Third      | !^         | vendor-1.2-1 : Second Vendor  |
    | third      | !^         | vendor-1.3-1 : Third Vendor   |
    | third      | !i^        | vendor-1.2-1 : Second Vendor  |
    | five       | !i^        | vendor-1.3-1 : Third Vendor   |
    | rd\ Vendor | !$         | vendor-1.2-1 : Second Vendor  |
    | rd\ vendor | !$         | vendor-1.3-1 : Third Vendor   |
    | rd\ vendor | !i$        | vendor-1.2-1 : Second Vendor  |
    | five       | !i$        | vendor-1.3-1 : Third Vendor   |
    | ^T.ird.*   | !=~        | vendor-1.2-1 : Second Vendor  |
    | ^t.ird.*   | !=~        | vendor-1.3-1 : Third Vendor   |
    | ^t.ird.*   | !i=~       | vendor-1.2-1 : Second Vendor  |
    | ^p.ird.*   | !i=~       | vendor-1.3-1 : Third Vendor   |
