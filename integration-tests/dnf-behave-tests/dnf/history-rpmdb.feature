Feature: DNF/Behave test rpmdb version


@bz1658120
Scenario: Compute rpmdb version in repeatable manner
  Given I use repository "dnf-ci-fedora"
   When I execute rpm with args "-U {context.dnf.fixturesdir}/repos/dnf-ci-fedora/noarch/setup-2.12.1-1.fc29.noarch.rpm"
   Then the exit code is 0
   Then I execute dnf with args "install dwm"
    And the exit code is 0
   Then I execute dnf with args "reinstall setup"
    And the exit code is 0
   Then History info rpmdb version changed
