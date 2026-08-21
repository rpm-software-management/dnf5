Feature: Upgrade using security advisories


@bz1770125
@bz1794644
Scenario: Upgrade packages with security issues
  Given I use repository "advisories-base"
   When I execute dnf with args "install labirinto"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | install       | labirinto-0:1.56.1-1.fc30.x86_64          |
  Given I use repository "advisories-updates"
   When I execute dnf with args "upgrade --security"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | labirinto-0:1.56.2-6.fc30.x86_64          |
   When I execute dnf with args "downgrade labirinto"
   Then the exit code is 0
   When I execute dnf with args "upgrade labirinto"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                                   |
        | upgrade       | labirinto-0:1.56.2-6.fc30.x86_64          |
