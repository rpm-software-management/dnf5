Feature: Protect running kernel


Background: Install fake kernel
  Given I use repository "protect-running-kernel"
    And I successfully execute dnf with args "install dnf-ci-kernel --exclude dnf-ci-obsolete"
    And I fake kernel release to "1.0"


@bz1698145
Scenario: Running kernel is protected
   When I execute dnf with args "remove dnf-ci-kernel"
   Then the exit code is 1
    And stderr is
        """
        Failed to resolve the transaction:
        Problem: The operation would result in removing of running kernel: dnf-ci-kernel-0:1.0-1.x86_64
        """


@bz1698145
Scenario: Running kernel is not protected with config protect_running_kernel=False
   When I execute dnf with args "remove dnf-ci-kernel --setopt=protect_running_kernel=False"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                             |
        | remove        | dnf-ci-kernel-0:1.0-1.x86_64        |
        | remove-unused | dnf-ci-systemd-0:1.0-1.x86_64       |


@bz1698145
Scenario: Running kernel is protected when in protected_packages even with config protect_running_kernel=False
   When I execute dnf with args "remove dnf-ci-kernel --setopt=protect_running_kernel=False --setopt=protected_packages=dnf-ci-kernel"
   Then the exit code is 1
    And stderr is
        """
        Failed to resolve the transaction:
        Problem: The operation would result in removing the following protected packages: dnf-ci-kernel
        """


@bz1698145
Scenario: Running kernel is protected against obsoleting
   When I execute dnf with args "install dnf-ci-obsolete"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        Problem: The operation would result in removing of running kernel: dnf-ci-kernel-0:1.0-1.x86_64
        You can try to add to command line:
          --skip-broken to skip uninstallable packages
        """


@bz1698145
Scenario: Running kernel is not protected against obsoleting with config protect_running_kernel=False
   When I execute dnf with args "install dnf-ci-obsolete --setopt=protect_running_kernel=False"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                             |
        | install       | dnf-ci-obsolete-0:1.0-1.x86_64      |
        | obsoleted     | dnf-ci-kernel-0:1.0-1.x86_64        |


@bz1698145
Scenario: Running kernel is protected against removal as conflict
   When I execute dnf with args "install dnf-ci-conflict --exclude dnf-ci-obsolete --allowerasing"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        Problem: problem with installed package
          - installed package dnf-ci-kernel-1.0-1.x86_64 conflicts with dnf-ci-kernel = 1.0-1 provided by dnf-ci-conflict-1.0-1.x86_64 from protect-running-kernel
          - package dnf-ci-conflict-1.0-1.x86_64 from protect-running-kernel conflicts with dnf-ci-kernel = 1.0-1 provided by dnf-ci-kernel-1.0-1.x86_64 from protect-running-kernel
          - conflicting requests
        You can try to add to command line:
          --skip-broken to skip uninstallable packages
        """

@bz1698145
Scenario: Running kernel is not protected against removal as conflict with config protect_running_kernel=False
   When I execute dnf with args "install dnf-ci-conflict --exclude dnf-ci-obsolete --allowerasing --setopt=protect_running_kernel=False"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                             |
        | install       | dnf-ci-conflict-0:1.0-1.x86_64      |
        | remove-dep    | dnf-ci-kernel-0:1.0-1.x86_64        |

@bz2178329
Scenario: If the oldest installed kernel is a running one it is not replaced on upgrade
   Given I configure dnf with
        | key                           | value                         |
        | installonlypkgs               | installonlypkg(dnf-ci-kernel) |
        | installonly_limit             | 3                             |
    And I use repository "protect-running-kernel-updates"
    And I execute dnf with args "install dnf-ci-kernel-2.0 dnf-ci-kernel-3.0"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install       | dnf-ci-kernel-0:2.0-1.x86_64      |
        | install       | dnf-ci-kernel-0:3.0-1.x86_64      |
   # Just like the tests above we need to exlude dnf-ci-obsolete, otherwise solver wants to
   # use it and obsolete the running kernel which fails because it is protected.
   # This test doesn't test obsoletes.
   When I execute dnf with args "upgrade --exclude dnf-ci-obsolete  dnf-ci-kernel"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | unchanged     | dnf-ci-kernel-0:1.0-1.x86_64      |
        | remove        | dnf-ci-kernel-0:2.0-1.x86_64      |
        | unchanged     | dnf-ci-kernel-0:3.0-1.x86_64      |
        | install       | dnf-ci-kernel-0:4.0-1.x86_64      |
