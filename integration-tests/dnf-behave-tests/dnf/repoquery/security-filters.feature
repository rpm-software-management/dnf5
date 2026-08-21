Feature: repoquery with security filters (--security, --bugfix, --newpackages, --enhancement)


Scenario: --security, --available with available security fix
Given I use repository "repoquery-security-filters"
 When I execute dnf with args "repoquery --security --available A"
 Then the exit code is 0
  And stderr is
  """
  <REPOSYNC>
  """
  And stdout is
  """
  A-0:2-2.x86_64
  A-0:3-3.x86_64
  """


Scenario: --security, --available with available security fix for installed pkg
Given I use repository "repoquery-security-filters"
  And I execute dnf with args "install A-1-1"
 When I execute dnf with args "repoquery --security --available A"
 Then the exit code is 0
  And stderr is
  """
  <REPOSYNC>
  """
  And stdout is
  """
  A-0:2-2.x86_64
  A-0:3-3.x86_64
  """


Scenario: --security, --installed with available security fix for installed pkg
Given I use repository "repoquery-security-filters"
  And I execute dnf with args "install A-1-1"
 When I execute dnf with args "repoquery --security --installed A"
 Then the exit code is 0
  And stdout is empty


Scenario: --security, --available with installed and available security fix
Given I use repository "repoquery-security-filters"
  And I execute dnf with args "install A-3-3"
 When I execute dnf with args "repoquery --security --available A"
 Then the exit code is 0
  And stderr is
  """
  <REPOSYNC>
  """
  And stdout is
  """
  A-0:2-2.x86_64
  A-0:3-3.x86_64
  """


Scenario: --security, --available with installed exact version of security fix
Given I use repository "repoquery-security-filters"
  And I execute dnf with args "install A-2-2"
 When I execute dnf with args "repoquery --security --available A"
 Then the exit code is 0
  And stderr is
  """
  <REPOSYNC>
  """
  And stdout is
  """
  A-0:2-2.x86_64
  A-0:3-3.x86_64
  """


# Because --installed is used dnf5 shows two A-2-2 packages one from
# system repo and one from repoquery-security-filters.
Scenario: --security with installed security fix
Given I use repository "repoquery-security-filters"
  And I execute dnf with args "install A-2-2"
 When I execute dnf with args "repoquery --security --available --installed A --info"
 Then the exit code is 0
 And stderr is
  """
  <REPOSYNC>
  """
 And stdout matches line by line
  """
  Name            : A
  Epoch           : 0
  Version         : 2
  Release         : 2
  Architecture    : x86_64
  Installed size  : 0.0   B
  Source          : A-2-2.src.rpm
  From repository : repoquery-security-filters
  Summary         : Testing advisory upgrade options
  URL             : None
  License         : Public Domain
  Description     : This package is part of testing security options
  Vendor          : dnf-ci-vendor2

  Name           : A
  Epoch          : 0
  Version        : 2
  Release        : 2
  Architecture   : x86_64
  Download size  : *
  Installed size : 0.0   B
  Source         : A-2-2.src.rpm
  Repository     : repoquery-security-filters
  Summary        : Testing advisory upgrade options
  URL            : None
  License        : Public Domain
  Description    : This package is part of testing security options
  Vendor         : dnf-ci-vendor2

  Name           : A
  Epoch          : 0
  Version        : 3
  Release        : 3
  Architecture   : x86_64
  Download size  : *
  Installed size : 0.0   B
  Source         : A-3-3.src.rpm
  Repository     : repoquery-security-filters
  Summary        : Testing advisory upgrade options
  URL            : None
  License        : Public Domain
  Description    : This package is part of testing security options
  Vendor         : dnf-ci-vendor3
  """


Scenario: --security, --available with advisory for a missing pkg and newer version present
Given I use repository "repoquery-security-filters"
 When I execute dnf with args "repoquery --security --available B"
 Then the exit code is 0
  And stderr is
  """
  <REPOSYNC>
  """
  And stdout is
  """
  B-0:2-2.x86_64
  """


Scenario: list all --security fixes when no pkg name is specified
Given I use repository "repoquery-security-filters"
 When I execute dnf with args "repoquery --security --available"
 Then the exit code is 0
  And stderr is
  """
  <REPOSYNC>
  """
  And stdout is
  """
  A-0:2-2.x86_64
  A-0:3-3.x86_64
  B-0:2-2.x86_64
  C-0:1-1.x86_64
  C-0:2-2.x86_64
  """


Scenario: package C has two versions where each has advisory
Given I use repository "repoquery-security-filters"
 When I execute dnf with args "repoquery --security C"
 Then the exit code is 0
  And stderr is
  """
  <REPOSYNC>
  """
  And stdout is
  """
  C-0:1-1.x86_64
  C-0:2-2.x86_64
  """


Scenario: repoquery shows a command line package when its present in an advisory
  Given I use repository "dnf-ci-security"
   When I execute dnf with args "install advisory_A-1.0-1 advisory_B-1.0-1"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                   |
        | install       | advisory_A-0:1.0-1.x86_64 |
        | install       | advisory_B-0:1.0-1.x86_64 |
   When I execute dnf with args "repoquery --cve CVE-002 {context.scenario.repos_location}/dnf-ci-security/x86_64/advisory_B-1.0-4.x86_64.rpm"
   Then the exit code is 0
    And stderr is
    """
    <REPOSYNC>
    """
    And stdout is
    """
    advisory_B-0:1.0-4.x86_64
    """


Scenario: repoquery filters out a command line package when its not present in an advisory
  Given I use repository "dnf-ci-security"
   When I execute dnf with args "install advisory_A-1.0-1 advisory_B-1.0-1"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                   |
        | install       | advisory_A-0:1.0-1.x86_64 |
        | install       | advisory_B-0:1.0-1.x86_64 |
   When I execute dnf with args "repoquery --cve CVE-002 {context.scenario.repos_location}/dnf-ci-security/x86_64/advisory_A-1.0-1.x86_64.rpm"
   Then the exit code is 0
    And stderr is
    """
    <REPOSYNC>
    """
    And stdout is empty
