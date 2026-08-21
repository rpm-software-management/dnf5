Feature: Autoremoval of unneeded packages

Scenario: Autoremoval of package which became non-required by others
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-thirdparty"
   When I execute dnf with args "install SuperRipper"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install       | SuperRipper-0:1.0-1.x86_64        |
        | install-dep   | abcde-0:2.9.2-1.fc29.noarch       |
        | install-dep   | wget-0:1.19.5-5.fc29.x86_64       |
        | install-weak  | flac-0:1.3.2-8.fc29.x86_64        |
        | install-weak  | FlacBetterEncoder-0:1.0-1.x86_64  |
   When I use repository "dnf-ci-thirdparty-updates"
    And I execute dnf with args "upgrade --no-best"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | upgrade       | SuperRipper-0:1.2-1.x86_64        |
        | broken        | SuperRipper-0:1.3-1.x86_64        |
   When I execute dnf with args "autoremove"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | remove        | abcde-0:2.9.2-1.fc29.noarch       |
        | remove        | flac-0:1.3.2-8.fc29.x86_64        |
        | remove        | wget-0:1.19.5-5.fc29.x86_64       |
        | remove        | FlacBetterEncoder-0:1.0-1.x86_64  |
