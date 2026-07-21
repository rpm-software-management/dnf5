# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
Feature: Enabling module streams - error handling


Background:
  Given I use repository "dnf-ci-fedora-modular-updates"


Scenario: Fail to enable a different stream of an already enabled module
   When I execute dnf with args "module enable nodejs:8"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package            |
        | module-stream-enable     | nodejs:8           |
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | nodejs    | enabled   | 8         |           |
   When I execute dnf with args "module enable nodejs:10"
   Then the exit code is 1
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | nodejs    | enabled   | 8         |           |
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        The operation would result in switching of module 'nodejs' stream '8' to stream '10'
        Error: It is not possible to switch enabled streams of a module unless explicitly enabled via configuration option module_stream_switch.
        """


# Missing module install command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
Scenario: Fail to install a different stream of an already enabled module
  Given I set dnf command to "dnf"
   When I execute dnf with args "module enable nodejs:8"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package            |
        | module-stream-enable     | nodejs:8           |
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | nodejs    | enabled   | 8         |           |
   When I execute dnf with args "module install nodejs:10/minimal --skip-broken"
   Then the exit code is 1
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | nodejs    | enabled   | 8         |           |
    And stderr is
        """
        <REPOSYNC>
        The operation would result in switching of module 'nodejs' stream '8' to stream '10'
        Error: It is not possible to switch enabled streams of a module unless explicitly enabled via configuration option module_stream_switch.
        """


# Missing module install command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
@bz1706215
Scenario: Fail to install a different stream of an already enabled module using @module:stream syntax
  Given I set dnf command to "dnf"
   When I execute dnf with args "module enable nodejs:8"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package            |
        | module-stream-enable     | nodejs:8           |
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | nodejs    | enabled   | 8         |           |
   When I execute dnf with args "install @nodejs:10/minimal --skip-broken"
   Then the exit code is 1
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | nodejs    | enabled   | 8         |           |
    And stderr is
        """
        <REPOSYNC>
        The operation would result in switching of module 'nodejs' stream '8' to stream '10'
        Error: It is not possible to switch enabled streams of a module unless explicitly enabled via configuration option module_stream_switch.
        """


@bz1814831
Scenario: Fail to enable a module stream when specifying only module
   When I execute dnf with args "module enable nodejs"
   Then the exit code is 1
    And Transaction is empty
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        Unable to resolve argument 'nodejs':
          - Argument 'nodejs' matches 4 streams ('10', '11', '12', '8') of module 'nodejs', but none of the streams are enabled or default.
        """


@bz1629655
Scenario: Fail to enable a module stream when specifying wrong version
   When I execute dnf with args "module enable nodejs:8:99"
   Then the exit code is 1
    And Transaction is empty
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        No match for argument: nodejs:8:99
        You can try to add to command line:
          --skip-unavailable to skip unavailable packages
        """


@bz1629655
Scenario: Fail to enable a non-existent module stream
   When I execute dnf with args "module enable nodejs:1"
   Then the exit code is 1
    And Transaction is empty
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        No match for argument: nodejs:1
        You can try to add to command line:
          --skip-unavailable to skip unavailable packages
        """


Scenario: Fail to enable a module stream when not specifying anything
   When I execute dnf with args "module enable"
   Then the exit code is 2
    And Transaction is empty
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
    And stderr is
        """
        Missing positional argument "specs" for command "enable". Add "--help" for more information about the arguments.
        """


@bz1581267
Scenario: Fail to enable a module stream when specifying more streams of the same module
   When I execute dnf with args "module enable nodejs:8 nodejs:10"
   Then the exit code is 1
    And Transaction is empty
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
    And stderr is
        """
        <REPOSYNC>
        Cannot enable multiple streams for module 'nodejs'
        """


# Module defaults from /etc/dnf/modules.defaults.d/ are not loaded
# https://github.com/rpm-software-management/dnf5/issues/1853
@xfail
@not.with_os=rhel__eq__8
Scenario: Enabling a stream depending on other than enabled stream should fail
  Given I use repository "dnf-ci-thirdparty-modular"
    And I create file "/etc/dnf/modules.defaults.d/defaults.yaml" with
        """
        ---
        document: modulemd-defaults
        version: 1
        data:
            module: beverage
            stream: soda
            profiles:
                default: [default]
        ...
        ---
        document: modulemd-defaults
        version: 1
        data:
            module: fluid
            stream: oil
            profiles:
                default: [default]
        ...
        """
   When I execute dnf with args "module enable fluid"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package                |
        | module-stream-enable     | fluid:oil              |
   When I execute dnf with args "module enable beverage:soda"
   Then the exit code is 1
    And stderr contains "Modular dependency problems:"
    And stderr contains "module beverage:soda:1:.x86_64 requires module\(fluid:water\), but none of the providers can be installed"


@not.with_os=rhel__eq__8
Scenario: Enabling a stream depending on a disabled stream should fail
  Given I use repository "dnf-ci-thirdparty-modular"
    And I create file "/etc/dnf/modules.defaults.d/defaults.yaml" with
        """
        ---
        document: modulemd-defaults
        version: 1
        data:
            module: beverage
            stream: soda
            profiles:
                default: [default]
        ...
        ---
        document: modulemd-defaults
        version: 1
        data:
            module: fluid
            stream: water
            profiles:
                default: [default]
        ...
        """
   When I execute dnf with args "module disable fluid"
   Then the exit code is 0
    And Transaction is following
        | Action                   | Package                |
        | module-disable           | fluid                  |
   When I execute dnf with args "module enable beverage:soda"
   Then the exit code is 1
    And stderr contains "Modular dependency problems:"
    And stderr contains "module beverage:soda:1:.x86_64 requires module\(fluid:water\), but none of the providers can be installed"
    And stderr contains "module fluid:water:1:.x86_64 is disabled"


# side-dish:chip requires fluid:oil
# beverage:beer requires fluid:water
@not.with_os=rhel__eq__8
Scenario: Enabling two modules both requiring different streams of another module
  Given I use repository "dnf-ci-thirdparty-modular"
   When I execute dnf with args "module enable side-dish:chips beverage:beer"
   Then the exit code is 1
    And stderr contains "Modular dependency problems:"
    And stderr contains "module side-dish:chips:1:.x86_64 requires module\(fluid:oil\), but none of the providers can be installed"
    And stderr contains "module beverage:beer:1:.x86_64 requires module\(fluid:water\), but none of the providers can be installed"


# beverage:beer requires fluid:water
@bz1651280
@not.with_os=rhel__eq__8
Scenario: Enabling module stream and another module requiring another stream
  Given I use repository "dnf-ci-thirdparty-modular"
   When I execute dnf with args "module enable fluid:oil beverage:beer"
   Then the exit code is 1
    And stderr contains "Modular dependency problems:"
    And stderr contains "module beverage:beer:1:.x86_64 requires module\(fluid:water\), but none of the providers can be installed"
