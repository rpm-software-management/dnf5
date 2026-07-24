Feature: Installing and removing multilib packages


Background: Use "install-remove-multilib" repo
  Given I use repository "install-remove-multilib"


@xfail
# https://bugzilla.redhat.com/show_bug.cgi?id=1745878
Scenario: Installing inferior arch with dependencies
 When I execute dnf with args "install packageB.i686"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                      |
      | install       | packageB-0:1.0-1.i686        |
      | install       | library-0:1.0-1.i686         |


@dnf5daemon
Scenario: Installing inferior arch with dependencies, in two steps
 When I execute dnf with args "install library.i686"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                      |
      | install       | library-0:1.0-1.i686         |
 When I execute dnf with args "install packageB.i686"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                      |
      | install       | packageB-0:1.0-1.i686        |


@xfail
# https://bugzilla.redhat.com/show_bug.cgi?id=1745878
@bz1745878
Scenario: Removing package of inferior arch also removes dependencies
 When I execute dnf with args "install packageA.x86_64"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                      |
      | install       | packageA-0:1.0-1.x86_64      |
      | install       | library-0:1.0-1.x86_64       |
 When I execute dnf with args "install packageB.i686"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                      |
      | install       | packageB-0:1.0-1.i686        |
      | install       | library-0:1.0-1.i686         |
 When I execute dnf with args "remove packageB"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                      |
      | remove        | packageB-0:1.0-1.i686        |
      | remove        | library-0:1.0-1.i686         |


@bz1782899
Scenario: Installing a package upgrade with best will not pull in secondary arch packages (non-trivial dependency tree)
Given I successfully execute dnf with args "install foo-1.0-1.x86_64"
 When I execute dnf with args "install foo --best"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                        |
      | upgrade       | foo-2.0-1.x86_64               |
      | upgrade       | python3-foo-2.0-1.x86_64       |
      | upgrade       | python3-foo-clibs-2.0-1.x86_64 |


@xfail
# https://github.com/openSUSE/libsolv/issues/404
Scenario: Installing an older version of x86_64 pulls in a newer version of i686 through dependencies
 When I execute dnf with args "install foo-1.0"
 Then Transaction is following
      | Action        | Package                        |
      | install       | foo-1.0-1.x86_64               |
      | install-dep   | python3-foo-1.0-1.x86_64       |
      | install-dep   | python3-foo-clibs-1.0-1.x86_64 |


@bz2172292
Scenario: undo install transaction when another arch is installed
Given I successfully execute dnf with args "install perduto.i686"
  And I successfully execute dnf with args "install perduto.x86_64"
 When I execute dnf with args "history undo last"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                        |
      | remove        | perduto-0:1.0-1.fc29.x86_64    |
