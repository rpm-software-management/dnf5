Feature: Tests for the debuginfo-install plugin

Background:
Given I use repository "debuginfo-install"

@bz1585137
Scenario: reports an error for a non-existent package
 When I execute dnf with args "debuginfo-install non-existent-package"
 Then the exit code is 1
  And stderr is
      """
      <REPOSYNC>
      Failed to resolve the transaction:
      No match for argument: non-existent-package
      You can try to add to command line:
        --skip-unavailable to skip unavailable packages
      """

Scenario: reports an error for a package without debuginfo
 When I execute dnf with args "debuginfo-install nodebug"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      Could not find debuginfo package for the following packages resolved from the argument 'nodebug': nodebug-0:1.0-1.x86_64
      Could not find debugsource package for the following packages resolved from the argument 'nodebug': nodebug-0:1.0-1.x86_64
      """

Scenario: installs latest version debuginfo for a not-installed package
 When I execute dnf with args "debuginfo-install foo"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | install       | foo-debuginfo-0:2.0-1.x86_64              |
      | install       | foo-debugsource-0:2.0-1.x86_64            |

@bz1586059 @bz1629412
Scenario: installs the same version of debuginfo for an installed package
Given I successfully execute dnf with args "install foo-1.0"
 When I execute dnf with args "debuginfo-install foo"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | install       | foo-debuginfo-0:1.0-1.x86_64              |
      | install       | foo-debugsource-0:1.0-1.x86_64            |

Scenario: with a -debugsource (or -debuginfo) package as an argument, installed version is not respected
Given I successfully execute dnf with args "install foo-1.0"
 When I execute dnf with args "debuginfo-install foo-debugsource"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | install       | foo-debugsource-0:2.0-1.x86_64            |

@bz1586059 @bz1629412
Scenario: installs the requested version of debuginfo even if a different base package is installed
Given I successfully execute dnf with args "install foo-2.0"
 When I execute dnf with args "debuginfo-install foo-1.0"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | install       | foo-debuginfo-0:1.0-1.x86_64              |
      | install       | foo-debugsource-0:1.0-1.x86_64            |

@bz1586059 @bz1629412
Scenario: installs the requested version of debuginfo (through a provide) even if a different base package is installed
Given I successfully execute dnf with args "install baz-1.0"
 When I execute dnf with args "debuginfo-install baz-2.0-provide"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | install       | baz-debuginfo-0:2.0-1.x86_64              |
      | install       | baz-debugsource-0:2.0-1.x86_64            |

Scenario: installs debuginfo when a -debuginfo package is provided
 When I execute dnf with args "debuginfo-install foo-debuginfo"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | install       | foo-debuginfo-0:2.0-1.x86_64              |
      | install-weak  | foo-debugsource-0:2.0-1.x86_64            |

@bz1586059 @bz1629412
Scenario: can't find the version of debuginfo for an installed package
Given I successfully execute dnf with args "install bar-2.0"
 When I execute dnf with args "debuginfo-install bar"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      Could not find debuginfo package for the following packages resolved from the argument 'bar': bar-0:2.0-1.x86_64
      Could not find debugsource package for the following packages resolved from the argument 'bar': bar-0:2.0-1.x86_64
      """

Scenario: can't find the requested version of debuginfo (multilib package, architectures can be mixed up)
 When I execute dnf with args "debuginfo-install baz-1.0"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      Could not find debuginfo package for the following packages resolved from the argument 'baz-1.0': baz-0:1.0-1.i686, baz-0:1.0-1.x86_64
      Could not find debugsource package for the following packages resolved from the argument 'baz-1.0': baz-0:1.0-1.i686, baz-0:1.0-1.x86_64
      """

Scenario: installs debuginfo with the same (secondary) architecture as the installed package
Given I successfully execute dnf with args "install baz.i686"
 When I execute dnf with args "debuginfo-install baz"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | install       | baz-debuginfo-0:2.0-1.i686                |
      | install       | baz-debugsource-0:2.0-1.i686              |

Scenario: installs debuginfo for multiple installed architectures
Given I successfully execute dnf with args "install baz.i686 baz.x86_64"
 When I execute dnf with args "debuginfo-install baz"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | install       | baz-debuginfo-0:2.0-1.i686                |
      | install       | baz-debuginfo-0:2.0-1.x86_64              |
      | install       | baz-debugsource-0:2.0-1.i686              |
      | install       | baz-debugsource-0:2.0-1.x86_64            |

Scenario: installs debuginfo for the latest version of multiple install-only packages
Given I successfully execute dnf with args "install kernel-1.0 kernel-2.0"
 When I execute dnf with args "debuginfo-install kernel"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | install       | kernel-debuginfo-0:2.0-1.x86_64           |
      | install       | kernel-debugsource-0:2.0-1.x86_64         |

Scenario: multiple packages, a glob that doesn't find debuginfo for the installed version
Given I successfully execute dnf with args "install foo-1.0 bar-2.0"
 When I execute dnf with args "debuginfo-install foo ba* nodebug"
 Then the exit code is 0
  And stderr contains "Could not find debuginfo package for the following packages resolved from the argument 'ba\*': bar-0:2\.0-1\.x86_64"
  And stderr contains "Could not find debugsource package for the following packages resolved from the argument 'ba\*': bar-0:2\.0-1\.x86_64"
  And stderr contains "Could not find debuginfo package for the following packages resolved from the argument 'nodebug': nodebug-0:1\.0-1\.x86_64"
  And stderr contains "Could not find debugsource package for the following packages resolved from the argument 'nodebug': nodebug-0:1\.0-1\.x86_64"
  And Transaction is following
      | Action        | Package                                   |
      | install       | baz-debuginfo-0:2.0-1.x86_64              |
      | install       | baz-debugsource-0:2.0-1.x86_64            |
      | install       | foo-debuginfo-0:1.0-1.x86_64              |
      | install       | foo-debugsource-0:1.0-1.x86_64            |

Scenario: a glob with version that overrides what is installed
Given I successfully execute dnf with args "install foo-1.0 bar-2.0"
 When I execute dnf with args "debuginfo-install foo ba*-1.0"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | install       | bar-debuginfo-0:1.0-1.x86_64              |
      | install       | bar-debugsource-0:1.0-1.x86_64            |
      | install       | foo-debuginfo-0:1.0-1.x86_64              |
      | install       | foo-debugsource-0:1.0-1.x86_64            |

Scenario: a package with a glob in version that overrides what is installed
Given I successfully execute dnf with args "install bar-2.0"
 When I execute dnf with args "debuginfo-install bar-1.*"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | install       | bar-debuginfo-0:1.0-1.x86_64              |
      | install       | bar-debugsource-0:1.0-1.x86_64            |

@bz1586084
Scenario: packages are upgraded according to the installed version of the base package
Given I successfully execute dnf with args "install bar-1.0"
Given I successfully execute dnf with args "debuginfo-install bar"
 Then Transaction is following
      | Action        | Package                                   |
      | install       | bar-debuginfo-0:1.0-1.x86_64              |
      | install       | bar-debugsource-0:1.0-1.x86_64            |
Given I successfully execute dnf with args "install bar-3.0"
 When I execute dnf with args "debuginfo-install bar"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | upgrade       | bar-debuginfo-0:3.0-1.x86_64              |
      | upgrade       | bar-debugsource-0:3.0-1.x86_64            |

@bz1586084
Scenario: debuginfo is upgraded according to the installed version of the base package, debugsource is not found
Given I successfully execute dnf with args "install bar-1.0"
Given I successfully execute dnf with args "debuginfo-install bar"
 Then Transaction is following
      | Action        | Package                                   |
      | install       | bar-debuginfo-0:1.0-1.x86_64              |
      | install       | bar-debugsource-0:1.0-1.x86_64            |
Given I successfully execute dnf with args "install bar-4.0"
 When I execute dnf with args "debuginfo-install bar"
 Then the exit code is 0
  And stderr contains "Could not find debugsource package for the following packages resolved from the argument 'bar': bar-0:4.0-1.x86_64"
  And Transaction is following
      | Action        | Package                                   |
      | upgrade       | bar-debuginfo-0:4.0-1.x86_64              |

@bz1532378
Scenario: debugsource is installed even if debuginfo is present on the system
Given I successfully execute dnf with args "install --setopt=install_weak_deps=False foo-debuginfo"
 When I execute dnf with args "debuginfo-install foo"
 Then the exit code is 0
 Then Transaction is following
      | Action        | Package                                   |
      | install       | foo-debugsource-0:2.0-1.x86_64            |

Scenario: installs latest version debuginfo of parent package for a subpackage
 When I execute dnf with args "debuginfo-install foo-subpackage"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | install       | foo-debuginfo-0:2.0-1.x86_64              |
      | install       | foo-debugsource-0:2.0-1.x86_64            |

@bz1586059 @bz1629412
Scenario: installs version of debuginfo for parent package of an installed subpackage
Given I successfully execute dnf with args "install foo-subpackage-1.0"
 When I execute dnf with args "debuginfo-install foo-subpackage"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | install       | foo-debuginfo-0:1.0-1.x86_64              |
      | install       | foo-debugsource-0:1.0-1.x86_64            |

@bz1586059 @bz1629412
Scenario: installs version of debuginfo for parent package of an installed subpackage
Given I successfully execute dnf with args "install foo-subpackage-1.0"
 When I execute dnf with args "debuginfo-install foo-subpackage-2.0"
 Then the exit code is 0
  And Transaction is following
      | Action        | Package                                   |
      | install       | foo-debuginfo-0:2.0-1.x86_64              |
      | install       | foo-debugsource-0:2.0-1.x86_64            |
