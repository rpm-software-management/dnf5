Feature: Test for dnf repoquery --unsatisfied


# options is not implemented: https://github.com/rpm-software-management/dnf5/issues/910
@xfail
@bz1750745
@no_installroot
@use.with_os=fedora__ge__30
@use.with_os=rhel__ge__8
Scenario: When kernel has unsatisfied dependencies, dnf repoquery --unsatisfied reports the problems and does not report "The operation would result in removing the following protected packages"
  Given I use repository "repoquery-unsatisfied"
   When I execute "dnf install dnf-ci-kernel --repofrompath=r,{context.dnf.repos[repoquery-unsatisfied].path} --repo=r -y --nogpgcheck --refresh"
   Then the exit code is 0
    And DNF Transaction is following
        | Action        | Package                             |
        | install       | dnf-ci-kernel-0:1.0-1.x86_64        |
        | install-dep   | dnf-ci-systemd-0:1.0-1.x86_64       |
   When I execute "rpm -e --nodeps dnf-ci-systemd"
   Then the exit code is 0
  Given I fake kernel release to "1.0"
   When I execute "dnf repoquery --unsatisfied"
   Then the exit code is 0
    And stdout contains "Problem: problem with installed package dnf-ci-kernel-1.0-1.x86_64"
    And stdout contains "- nothing provides dnf-ci-systemd needed by dnf-ci-kernel-1.0-1.x86_64"
