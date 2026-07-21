Feature: Listing available updates using the dnf advisory command


Background:
  Given I use repository "dnf-ci-fedora"


# @dnf5daemon
Scenario: Listing available updates
   When I execute dnf with args "install glibc flac"
   Then Transaction is following
        | Action        | Package                                  |
        | install       | glibc-0:2.28-9.fc29.x86_64               |
        | install       | flac-0:1.3.2-8.fc29.x86_64               |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch             |
        | install-dep   | basesystem-0:11-6.fc29.noarch            |
        | install-dep   | filesystem-0:3.9-2.fc29.x86_64           |
        | install-dep   | glibc-all-langpacks-0:2.28-9.fc29.x86_64 |
        | install-dep   | glibc-common-0:2.28-9.fc29.x86_64        |
    And the exit code is 0
  Given I use repository "dnf-ci-fedora-updates"
   Then I execute dnf with args "advisory list"
    And the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Name                   Type        Severity                   Package              Issued
        FEDORA-2018-318f184000 bugfix      none     glibc-2.28-26.fc29.x86_64 2019-01-17 00:00:00
        FEDORA-2999:002-02     enhancement Moderate  flac-1.3.3-8.fc29.x86_64 2019-01-17 00:00:00
        """


Scenario: advisory summary (when there's nothing to report)
   When I execute dnf with args "install glibc flac"
   Then the exit code is 0
   When I execute dnf with args "advisory summary"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Available advisory information summary:
        Security    : 0
          Critical  : 0
          Important : 0
          Moderate  : 0
          Low       : 0
          Other     : 0
        Bugfix      : 0
        Enhancement : 0
        Other       : 0
        """


Scenario: advisory summary --available (when there is an available update)
   When I execute dnf with args "install glibc flac"
   Then the exit code is 0
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "advisory summary --available"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Available advisory information summary:
        Security    : 0
          Critical  : 0
          Important : 0
          Moderate  : 0
          Low       : 0
          Other     : 0
        Bugfix      : 1
        Enhancement : 1
        Other       : 0
        """


# @dnf5daemon
Scenario: advisory info
   When I execute dnf with args "install glibc flac"
   Then the exit code is 0
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "advisory info --available"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Name        : FEDORA-2018-318f184000
        Title       : glibc bug fix
        Severity    : none
        Type        : bugfix
        Status      : final
        Vendor      : secresponseteam@foo.bar
        Issued      : 2019-01-17 00:00:00
        Description : Fix some stuff
        Message     :
        Rights      :
        Reference   :
          Title     : 222
          Id        : 222
          Type      : bugzilla
          Url       : https://foobar/foobarupdate_1
        Reference   :
          Title     : CVE-2999
          Id        : 2999
          Type      : cve
          Url       : https://foobar/foobarupdate_1
        Reference   :
          Title     : CVE-2999
          Id        : CVE-2999
          Type      : cve
          Url       : https://foobar/foobarupdate_1
        Collection  :
          Packages  : glibc-2.28-26.fc29.x86_64

        Name        : FEDORA-2999:002-02
        Title       : flac enhacements
        Severity    : Moderate
        Type        : enhancement
        Status      : final
        Vendor      : secresponseteam@foo.bar
        Issued      : 2019-01-17 00:00:00
        Description : Enhance some stuff
        Message     :
        Rights      :
        Reference   :
          Title     : update_1
          Id        : 1
          Type      : self
          Url       : https://foobar/foobarupdate_1
        Collection  :
          Packages  : flac-1.3.3-8.fc29.x86_64
        """


# @dnf5daemon
Scenario: advisory info security (when there's nothing to report)
   When I execute dnf with args "install glibc flac"
   Then the exit code is 0
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "advisory info --security"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        No advisory found matching the specified filters.
        """
    And stdout is empty


# @dnf5daemon
Scenario: advisory list
   When I execute dnf with args "install glibc flac"
   Then the exit code is 0
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "advisory list"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Name                   Type        Severity                   Package              Issued
        FEDORA-2018-318f184000 bugfix      none     glibc-2.28-26.fc29.x86_64 2019-01-17 00:00:00
        FEDORA-2999:002-02     enhancement Moderate  flac-1.3.3-8.fc29.x86_64 2019-01-17 00:00:00
        """


# @dnf5daemon
Scenario: advisory list all security
  Given I use repository "dnf-ci-fedora-updates-testing"
   When I execute dnf with args "install glibc flac CQRlib"
   Then the exit code is 0
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "advisory list --all --security"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Name                   Type     Severity                     Package              Issued
        FEDORA-2018-318f184113 security Moderate CQRlib-1.1.2-16.fc29.x86_64 2019-01-20 00:00:00
        """


# @dnf5daemon
Scenario: advisory list updates
   When I execute dnf with args "install glibc flac"
   Then the exit code is 0
  Given I use repository "dnf-ci-fedora-updates"
   Then I execute dnf with args "upgrade glibc flac"
    And the exit code is 0
  Given I use repository "dnf-ci-fedora-updates-testing"
   When I execute dnf with args "advisory list --updates"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Name                   Type        Severity                  Package              Issued
        FEDORA-2018-318f184112 enhancement Moderate flac-1.4.0-1.fc29.x86_64 2019-01-19 00:00:00
        FEDORA-2999:002-02     enhancement Moderate flac-1.3.3-8.fc29.x86_64 2019-01-17 00:00:00
        """


# @dnf5daemon
Scenario: advisory list installed
   When I execute dnf with args "install glibc flac"
   Then the exit code is 0
  Given I use repository "dnf-ci-fedora-updates"
   Then I execute dnf with args "upgrade glibc flac"
    And the exit code is 0
  Given I use repository "dnf-ci-fedora-updates-testing"
   When I execute dnf with args "advisory list --installed"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Name                   Type   Severity                   Package              Issued
        FEDORA-2018-318f184000 bugfix none     glibc-2.28-26.fc29.x86_64 2019-01-17 00:00:00
        """


# @dnf5daemon
Scenario: advisory list available enhancement
   When I execute dnf with args "install glibc flac"
   Then the exit code is 0
  Given I use repository "dnf-ci-fedora-updates"
  Given I use repository "dnf-ci-fedora-updates-testing"
   When I execute dnf with args "advisory list --available --enhancement"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        <REPOSYNC>
        Name                   Type        Severity                  Package              Issued
        FEDORA-2018-318f184112 enhancement Moderate flac-1.4.0-1.fc29.x86_64 2019-01-19 00:00:00
        FEDORA-2999:002-02     enhancement Moderate flac-1.3.3-8.fc29.x86_64 2019-01-17 00:00:00
        """


# @dnf5daemon
Scenario: advisory list all bugfix
   When I execute dnf with args "install glibc flac"
   Then the exit code is 0
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "advisory list --all --bugfix"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        <REPOSYNC>
        Name                   Type   Severity                   Package              Issued
        FEDORA-2018-318f184000 bugfix none     glibc-2.28-26.fc29.x86_64 2019-01-17 00:00:00
        """


# @dnf5daemon
Scenario Outline: advisory list updates plus <option>
   When I execute dnf with args "install glibc flac"
   Then the exit code is 0
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "advisory list --updates <option> <value>"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Name                   Type   Severity                   Package              Issued
        FEDORA-2018-318f184000 bugfix none     glibc-2.28-26.fc29.x86_64 2019-01-17 00:00:00
        """

Examples:
        | option | value                  |
        | --bzs  | 222                    |
        | --cves | 2999                   |
        | --cves | CVE-2999               |
        |        | FEDORA-2018-318f184000 |


# @dnf5daemon
Scenario: advisory info <advisory>
   When I execute dnf with args "install glibc flac"
   Then the exit code is 0
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "advisory info FEDORA-2018-318f184000"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Name        : FEDORA-2018-318f184000
        Title       : glibc bug fix
        Severity    : none
        Type        : bugfix
        Status      : final
        Vendor      : secresponseteam@foo.bar
        Issued      : 2019-01-17 00:00:00
        Description : Fix some stuff
        Message     :
        Rights      :
        Reference   :
          Title     : 222
          Id        : 222
          Type      : bugzilla
          Url       : https://foobar/foobarupdate_1
        Reference   :
          Title     : CVE-2999
          Id        : 2999
          Type      : cve
          Url       : https://foobar/foobarupdate_1
        Reference   :
          Title     : CVE-2999
          Id        : CVE-2999
          Type      : cve
          Url       : https://foobar/foobarupdate_1
        Collection  :
          Packages  : glibc-2.28-26.fc29.x86_64
        """


# @dnf5daemon
Scenario: advisory info <advisory-with-respin-suffix>
   When I execute dnf with args "install glibc flac"
   Then the exit code is 0
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "advisory info FEDORA-2999:002-02"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Name        : FEDORA-2999:002-02
        Title       : flac enhacements
        Severity    : Moderate
        Type        : enhancement
        Status      : final
        Vendor      : secresponseteam@foo.bar
        Issued      : 2019-01-17 00:00:00
        Description : Enhance some stuff
        Message     :
        Rights      :
        Reference   :
          Title     : update_1
          Id        : 1
          Type      : self
          Url       : https://foobar/foobarupdate_1
        Collection  :
          Packages  : flac-1.3.3-8.fc29.x86_64
        """


@bz1750528
Scenario: advisory lists advisories referencing CVE
  Given I successfully execute dnf with args "install glibc flac"
    And I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "advisory list --with-cve"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        CVE      Type   Severity                   Package              Issued
        2999     bugfix none     glibc-2.28-26.fc29.x86_64 2019-01-17 00:00:00
        CVE-2999 bugfix none     glibc-2.28-26.fc29.x86_64 2019-01-17 00:00:00
        """


Scenario: advisory lists advisories referencing bugzilla
  Given I successfully execute dnf with args "install glibc flac"
    And I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "advisory list --with-bz"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Bugzilla Type   Severity                   Package              Issued
        222      bugfix none     glibc-2.28-26.fc29.x86_64 2019-01-17 00:00:00
        """


@bz1728004
Scenario: advisory show <advisory> of the running kernel after a kernel update
   # install older kernel
  Given I use repository "dnf-ci-fedora-updates"
    And I fake kernel release to "4.18.16-300.fc29.x86_64"
    And I successfully execute dnf with args "install kernel-4.18.16-300.fc29"
   Then Transaction is following
        | Action        | Package                                  |
        | install       | kernel-0:4.18.16-300.fc29.x86_64         |
        | install-dep   | kernel-core-0:4.18.16-300.fc29.x86_64    |
        | install-dep   | kernel-modules-0:4.18.16-300.fc29.x86_64 |
   # updated kernel is available
   When I execute dnf with args "advisory list --contains-pkgs=kernel"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Name                   Type   Severity                        Package              Issued
        FEDORA-2019-348e185000 bugfix Moderate kernel-4.19.15-300.fc29.x86_64 2019-01-17 00:00:00
        """
  Given I successfully execute dnf with args "upgrade kernel"
   Then Transaction is following
        | Action        | Package                                  |
        | install       | kernel-0:4.19.15-300.fc29.x86_64         |
        | install-dep   | kernel-core-0:4.19.15-300.fc29.x86_64    |
        | install-dep   | kernel-modules-0:4.19.15-300.fc29.x86_64 |
   # updated kernel is installed, but the running kernel is still the older one
   When I execute dnf with args "advisory list --contains-pkgs=kernel"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Name                   Type   Severity                        Package              Issued
        FEDORA-2019-348e185000 bugfix Moderate kernel-4.19.15-300.fc29.x86_64 2019-01-17 00:00:00
        """
  Given I fake kernel release to "4.19.15-300.fc29.x86_64"
   # updated kernel is installed and running
   When I execute dnf with args "advisory list --contains-pkgs=kernel"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is empty


@bz1801092
Scenario: advisory lists advisories referencing CVE with dates
  Given I successfully execute dnf with args "install glibc flac"
    And I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "advisory list --with-cve"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        CVE      Type   Severity                   Package              Issued
        2999     bugfix none     glibc-2.28-26.fc29.x86_64 2019-01-17 00:00:00
        CVE-2999 bugfix none     glibc-2.28-26.fc29.x86_64 2019-01-17 00:00:00
        """


# @dnf5daemon
Scenario: advisory lists advisories with custom type and severity
  Given I use repository "advisories-base"
    And I execute dnf with args "install labirinto"
    And I use repository "advisories-updates"
   When I execute dnf with args "advisory list"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Name                   Type        Severity                               Package              Issued
        FEDORA-2019-57b5902ed1 security    Critical        labirinto-1.56.2-6.fc30.x86_64 2019-09-15 01:34:29
        FEDORA-2019-f4eb34cf4c security    Moderate        labirinto-1.56.2-1.fc30.x86_64 2019-05-12 01:21:43
        FEDORA-2022-2222222222 custom_type custom_severity labirinto-1.56.2-6.fc30.x86_64 2019-09-15 01:34:29
        FEDORA-2022-2222222223 security    custom_severity labirinto-1.56.2-6.fc30.x86_64 2019-09-15 01:34:29
        FEDORA-2022-2222222224 custom_type Critical        labirinto-1.56.2-6.fc30.x86_64 2019-09-15 01:34:29
        """


Scenario: advisory prints info for advisories with custom type and severity
  Given I use repository "advisories-base"
    And I execute dnf with args "install labirinto"
    And I use repository "advisories-updates"
   When I execute dnf with args "advisory info"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Name        : FEDORA-2019-f4eb34cf4c
        Title       : labirinto-1.56.2-1.fc30
        Severity    : Moderate
        Type        : security
        Status      : stable
        Vendor      : updates@fedoraproject.org
        Issued      : 2019-05-12 01:21:43
        Description : GNOME 3.32.2
        Message     :
        Rights      : Copyright (C) 2020 Red Hat, Inc. and others.
        Reference   :
          Title     : [abrt] epiphany: ephy_suggestion_get_unescaped_title(): epiphany-search-provider killed by SIGABRT
          Id        : 1696529
          Type      : bugzilla
          Url       : https://bugzilla.redhat.com/show_bug.cgi?id=1696529
        Collection  :
          Packages  : labirinto-1.56.2-1.fc30.i686
                    : labirinto-1.56.2-1.fc30.x86_64
                    : labirinto-1.56.2-1.fc30.src

        Name        : FEDORA-2019-57b5902ed1
        Title       : labirinto-1.56.2-6.fc30 mozjs60-60.9.0-2.fc30 polkit-0.116-2.fc30
        Severity    : Critical
        Type        : security
        Status      : stable
        Vendor      : updates@fedoraproject.org
        Issued      : 2019-09-15 01:34:29
        Description : mozjs60 60.9.0, including various security, stability and regression fixes from Firefox 60.9.0 ESR. For details, see https://www.mozilla.org/en-US/firefox/60.9.0/releasenotes/
        Message     :
        Rights      : Copyright (C) 2020 Red Hat, Inc. and others.
        Collection  :
          Packages  : labirinto-1.56.2-6.fc30.i686
                    : labirinto-1.56.2-6.fc30.src
                    : labirinto-1.56.2-6.fc30.x86_64

        Name        : FEDORA-2022-2222222222
        Title       : labirinto-1.56.2-6.fc30 mozjs60-60.9.0-2.fc30 polkit-0.116-2.fc30
        Severity    : custom_severity
        Type        : custom_type
        Status      : stable
        Vendor      : updates@fedoraproject.org
        Issued      : 2019-09-15 01:34:29
        Description : advisory with custom type and seveirity
        Message     :
        Rights      : Copyright (C) 2020 Red Hat, Inc. and others.
        Collection  :
          Packages  : labirinto-1.56.2-6.fc30.i686
                    : labirinto-1.56.2-6.fc30.src
                    : labirinto-1.56.2-6.fc30.x86_64

        Name        : FEDORA-2022-2222222223
        Title       : labirinto-1.56.2-6.fc30 mozjs60-60.9.0-2.fc30 polkit-0.116-2.fc30
        Severity    : custom_severity
        Type        : security
        Status      : stable
        Vendor      : updates@fedoraproject.org
        Issued      : 2019-09-15 01:34:29
        Description : advisory with custom seveirity
        Message     :
        Rights      : Copyright (C) 2020 Red Hat, Inc. and others.
        Collection  :
          Packages  : labirinto-1.56.2-6.fc30.i686
                    : labirinto-1.56.2-6.fc30.src
                    : labirinto-1.56.2-6.fc30.x86_64

        Name        : FEDORA-2022-2222222224
        Title       : labirinto-1.56.2-6.fc30 mozjs60-60.9.0-2.fc30 polkit-0.116-2.fc30
        Severity    : Critical
        Type        : custom_type
        Status      : stable
        Vendor      : updates@fedoraproject.org
        Issued      : 2019-09-15 01:34:29
        Description : advisory with custom type
        Message     :
        Rights      : Copyright (C) 2020 Red Hat, Inc. and others.
        Collection  :
          Packages  : labirinto-1.56.2-6.fc30.i686
                    : labirinto-1.56.2-6.fc30.src
                    : labirinto-1.56.2-6.fc30.x86_64
        """


Scenario: advisory prints summary of advisories with custom type and severity
  Given I use repository "advisories-base"
    And I execute dnf with args "install labirinto"
    And I use repository "advisories-updates"
   When I execute dnf with args "updateinfo summary"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Available advisory information summary:
        Security    : 3
          Critical  : 1
          Important : 0
          Moderate  : 1
          Low       : 0
          Other     : 1
        Bugfix      : 0
        Enhancement : 0
        Other       : 2
        """


# @dnf5daemon
Scenario: advisory for x86_64 package is not shown as installed when noarch version of the pkg is installed
Given I use repository "updateinfo"
  And I execute dnf with args "install A-2-2.noarch"
# The test requires that advisory for A-2-2.x86_64 is available
 When I execute dnf with args "advisory list --installed"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is empty


@xfail
# This has been broken in dnf4 for a long time, it will be fixed for dnf5
# It is currenly failing on dnf5 as well but should be resolved by: https://issues.redhat.com/browse/RHELPLAN-133820
Scenario: updateinfo --updates with advisory for obsoleter when obsoleted installed
Given I use repository "security-upgrade"
  And I execute dnf with args "install C-1-1"
 When I execute dnf with args "updateinfo list --updates"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      Name         Type     Severity      Package              Issued
      DNF-D-2022-9 security          D-1-1.x86_64 1970-01-01 00:00:00
      """


Scenario: advisory --updates with advisory for obsoleter when obsoleted installed
Given I use repository "security-upgrade"
  And I execute dnf with args "install C-1-1"
  # Make sure D-1-1 is available and obsoletes C since we are testing we don't list it.
  # There also should be an available advisory for it. (We cannot verify that here because the advisory command is bugged when dealing with obsoletes)
  And I successfully execute dnf with args "repoquery D-1-1.x86_64 --obsoletes"
 Then stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      C
      """
 When I execute dnf with args "advisory list --updates --setopt=obsoletes=false"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is empty


@xfail
# This has been broken in dnf4 for a long time, it will be fixed for dnf5
# It is currenly failing on dnf5 as well but should be resolved by: https://issues.redhat.com/browse/RHELPLAN-133820
Scenario: advisory for noarch package is shown as an upgrade when lower arch version of the pkg is installed
Given I use repository "security-upgrade"
  And I execute dnf with args "install change-arch-noarch-1-1.noarch"
 When I execute dnf with args "updateinfo list --updates"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      Name       Type     Severity                       Package              Issued
      DNF-2019-4 security          change-arch-noarch-2-2.x86_64 1970-01-01 00:00:00
      """


# @dnf5daemon
Scenario: advisory list --contains-pkgs doesn't list other packages (including running kernel)
Given I successfully execute dnf with args "install kernel flac glibc"
  And I fake kernel release to "4.18.16-300.fc29.x86_64"
  And I use repository "dnf-ci-fedora-updates"
 When I execute dnf with args "advisory list --contains-pkgs=flac"
 Then stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      Name               Type        Severity                  Package              Issued
      FEDORA-2999:002-02 enhancement Moderate flac-1.3.3-8.fc29.x86_64 2019-01-17 00:00:00
      """
