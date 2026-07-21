Feature: Autoremoval of unneeded packages

Scenario: Remove with --setopt=clean_requirements_on_remove=True
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-thirdparty"
    When I execute dnf with args "install abcde"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install       | abcde-0:2.9.2-1.fc29.noarch       |
        | install-dep   | wget-0:1.19.5-5.fc29.x86_64       |
        | install-weak  | flac-0:1.3.2-8.fc29.x86_64        |
        | install-weak  | FlacBetterEncoder-0:1.0-1.x86_64  |
   When I execute dnf with args "remove abcde --setopt=clean_requirements_on_remove=True"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | remove        | abcde-0:2.9.2-1.fc29.noarch       |
        | remove-unused | flac-0:1.3.2-8.fc29.x86_64        |
        | remove-unused | wget-0:1.19.5-5.fc29.x86_64       |
        | remove-unused | FlacBetterEncoder-0:1.0-1.x86_64  |

Scenario: Remove with --setopt=clean_requirements_on_remove=False
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-thirdparty"
    When I execute dnf with args "install abcde"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install       | abcde-0:2.9.2-1.fc29.noarch       |
        | install-dep   | wget-0:1.19.5-5.fc29.x86_64       |
        | install-weak  | flac-0:1.3.2-8.fc29.x86_64        |
        | install-weak  | FlacBetterEncoder-0:1.0-1.x86_64  |
   When I execute dnf with args "remove abcde --setopt=clean_requirements_on_remove=False"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | remove        | abcde-0:2.9.2-1.fc29.noarch       |

Scenario: Remove with --no-autoremove
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-thirdparty"
    When I execute dnf with args "install abcde"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install       | abcde-0:2.9.2-1.fc29.noarch       |
        | install-dep   | wget-0:1.19.5-5.fc29.x86_64       |
        | install-weak  | flac-0:1.3.2-8.fc29.x86_64        |
        | install-weak  | FlacBetterEncoder-0:1.0-1.x86_64  |
   When I execute dnf with args "remove abcde --no-autoremove"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | remove        | abcde-0:2.9.2-1.fc29.noarch       |
