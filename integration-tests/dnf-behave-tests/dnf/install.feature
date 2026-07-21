Feature: RPM installation tests

Background: Set repositories
Given I use repository "simple-base"

# https://github.com/rpm-software-management/dnf5/issues/1620
Scenario: Install an RPM with `destdir` option set
 When I execute dnf with args "install labirinto --setopt=destdir={context.dnf.tempdir}/SomeDestination"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                       |
      | install       | labirinto-0:1.0-1.fc29.x86_64 |
 When I execute "find -type f -name '*.rpm'" in "{context.dnf.tempdir}/SomeDestination"
 Then stdout is empty


Scenario: RPMs downloaded with `keepcache` and `destdir` options set are not removed
 When I execute dnf with args "install labirinto --setopt=destdir={context.dnf.tempdir}/SomeDestination --setopt=keepcache=true"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                       |
      | install       | labirinto-0:1.0-1.fc29.x86_64 |
 When I execute "find -type f -name '*.rpm'" in "{context.dnf.tempdir}/SomeDestination"
 Then stdout matches line by line
      """
      \./labirinto-1\.0-1\.fc29\.x86_64\.rpm
      """


Scenario: Install an RPM with `destdir` option set and `--downloadonly` switch works
 When I execute dnf with args "install labirinto --setopt=destdir={context.dnf.tempdir}/SomeDestination --downloadonly"
 Then the exit code is 0
  And DNF Transaction is following
      | Action        | Package                       |
      | install       | labirinto-0:1.0-1.fc29.x86_64 |
  And RPMDB Transaction is empty
 When I execute "find -type f -name '*.rpm'" in "{context.dnf.tempdir}/SomeDestination"
 Then stdout matches line by line
      """
      \./labirinto-1\.0-1\.fc29\.x86_64\.rpm
      """
