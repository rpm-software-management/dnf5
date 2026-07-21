Feature: Help command


Scenario: General help
   When I execute dnf with args "--help"
   Then the exit code is 0
    And stdout contains "Software Management Commands"
   When I execute dnf with args "--unknown-option"
   Then the exit code is 2
    And stdout is empty
    And stderr is
    """
    Unknown argument "--unknown-option" for command "dnf5". Add "--help" for more information about the arguments.
    """
   When I execute dnf with args "help"
   Then the exit code is 2
    And stdout is empty
    And stderr is
    """
    Unknown argument "help" for command "dnf5". Add "--help" for more information about the arguments.
    It could be a command provided by a plugin, try: dnf5 install 'dnf5-command(help)'
    """
   When I execute dnf with args "unknown-command"
   Then the exit code is 2
    And stdout is empty
    And stderr is
    """
    Unknown argument "unknown-command" for command "dnf5". Add "--help" for more information about the arguments.
    It could be a command provided by a plugin, try: dnf5 install 'dnf5-command(unknown-command)'
    """


Scenario: Command help
   When I execute dnf with args "install --help"
   Then the exit code is 0
    And stdout contains "dnf5 \[GLOBAL OPTIONS\] install \[OPTIONS\] \[ARGUMENTS\]"
   When I execute dnf with args "install --unknown-option"
   Then the exit code is 2
    And stderr is
    """
    Unknown argument "--unknown-option" for command "install". Add "--help" for more information about the arguments.
    """
