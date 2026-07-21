@dnf5daemon
Feature: Upgrade dependency of another RPM


# To reproduce the issue from the bug you need:
# - two packages in two versions (here labirinto and labirinto-libs)
# - package labirinto requires labirinto-libs on specific version
#       labirinto-1 requires labirinto-libs-1
#       labirinto-2 requires labirinto-libs-2
# - best must be set to False (Fedora default)
# - attempt to upgrade dependent package (here labirinto-libs)
@bz2142257
Scenario: Upgrade dependency of another RPM with best=False
  Given I configure dnf with
        | key   | value |
        | best  | False |
  Given I use repository "upgrade-dependent"
    And I successfully execute dnf with args "install labirinto-1.0-1"
   Then Transaction is following
        | Action           | Package                       |
        | install          | labirinto-0:1.0-1.noarch      |
        | install-dep      | labirinto-libs-0:1.0-1.noarch |
   When I execute dnf with args "upgrade labirinto-libs"
   Then the exit code is 0
    And Transaction is following
        | Action           | Package                       |
        | upgrade          | labirinto-0:2.0-1.noarch      |
        | upgrade          | labirinto-libs-0:2.0-1.noarch |
