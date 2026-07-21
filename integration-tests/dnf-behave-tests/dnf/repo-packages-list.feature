@xfail
# repository-packages list is missing: https://github.com/rpm-software-management/dnf5/issues/951
Feature: repository-packages list installed packages from repository


Scenario: List related packages to repo "dnf-ci-fedora"
Given I use repository "dnf-ci-fedora"
 When I execute dnf with args "install basesystem"
 Then the exit code is 0
 When I execute dnf with args "repository-packages -q dnf-ci-fedora list"
 Then the exit code is 0
 Then stdout section "Installed Packages" contains "setup.noarch\s+2.12.1-1.fc29\s+@dnf-ci-fedora"
 Then stdout section "Installed Packages" contains "basesystem.noarch\s+11-6.fc29\s+@dnf-ci-fedora"
 Then stdout section "Installed Packages" contains "filesystem.x86_64\s+3.9-2.fc29\s+@dnf-ci-fedora"
 Then stdout section "Available Packages" contains "glibc-common.x86_64\s+2.28-9.fc29\s+dnf-ci-fedora"
 Then stdout section "Available Packages" contains "glibc.x86_64\s+2.28-9.fc29\s+dnf-ci-fedora"
 Then stdout section "Available Packages" contains "glibc-all-langpacks.x86_64\s+2.28-9.fc29\s+dnf-ci-fedora"


Scenario: List installed packages from repo "dnf-ci-fedora"
Given I use repository "dnf-ci-fedora"
 When I execute dnf with args "install glibc"
 Then the exit code is 0
 When I execute dnf with args "repository-packages -q dnf-ci-fedora list --installed"
 Then the exit code is 0
 Then stdout section "Installed Packages" contains "glibc-common.x86_64\s+2.28-9.fc29\s+@dnf-ci-fedora"
 Then stdout section "Installed Packages" contains "glibc.x86_64\s+2.28-9.fc29\s+@dnf-ci-fedora"
 Then stdout section "Installed Packages" contains "glibc-all-langpacks.x86_64\s+2.28-9.fc29\s+@dnf-ci-fedora"


Scenario: List available packages from repo "dnf-ci-fedora"
Given I use repository "dnf-ci-fedora"
 When I execute dnf with args "repository-packages -q dnf-ci-fedora list --available"
 Then the exit code is 0
 Then stdout section "Available Packages" contains "glibc-common.x86_64\s+2.28-9.fc29\s+dnf-ci-fedora"
 Then stdout section "Available Packages" contains "glibc.x86_64\s+2.28-9.fc29\s+dnf-ci-fedora"
 Then stdout section "Available Packages" contains "glibc-all-langpacks.x86_64\s+2.28-9.fc29\s+dnf-ci-fedora"
 Then stdout section "Available Packages" contains "setup.noarch\s+2.12.1-1.fc29\s+dnf-ci-fedora"
 Then stdout section "Available Packages" contains "basesystem.noarch\s+11-6.fc29\s+dnf-ci-fedora"


Scenario: List packages from repo "dnf-ci-fedora-updates" that obsolete some installed packages
Given I use repository "dnf-ci-fedora"
 When I execute dnf with args "install glibc"
 Then the exit code is 0
Given I use repository "dnf-ci-fedora-updates"
 When I execute dnf with args "repository-packages -q dnf-ci-fedora-updates list --obsoletes"
 Then the exit code is 0
 Then stdout section "Obsoleting Packages" contains "glibc.x86_64\s+2.28-26.fc29\s+dnf-ci-fedora-updates"
 Then stdout section "Obsoleting Packages" contains "\sglibc.x86_64\s+2.28-9.fc29\s+@dnf-ci-fedora"


Scenario: List packages from repo "dnf-ci-fedora-updates" that upgrade some installed packages
Given I use repository "dnf-ci-fedora"
 When I execute dnf with args "install glibc"
 Then the exit code is 0
Given I use repository "dnf-ci-fedora-updates"
 When I execute dnf with args "repository-packages -q dnf-ci-fedora-updates list --upgrades"
 Then the exit code is 0
 Then stdout section "Available Upgrades" contains "glibc.x86_64\s+2.28-26.fc29\s+dnf-ci-fedora-updates"
 Then stdout section "Available Upgrades" contains "glibc-all-langpacks.x86_64\s+2.28-26.fc29\s+dnf-ci-fedora-updates"
 Then stdout section "Available Upgrades" contains "glibc-common.x86_64\s+2.28-26.fc29\s+dnf-ci-fedora-updates"
