Feature: --setopt option


Background: Use repos setopt and setopt.ext
  Given I use repository "setopt"
    And I use repository "setopt.ext"


# setopt repo contains: wget
# setopt.ext repo contains: flac, wget


Scenario: Without --setopt option, packages wget and flac are available
   When I execute dnf with args "repoquery"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        flac-0:1.0-1.fc29.src
        flac-0:1.0-1.fc29.x86_64
        flac-libs-0:1.0-1.fc29.x86_64
        wget-0:1.0-1.fc29.src
        wget-0:1.0-1.fc29.x86_64
        """

@bz1746349
Scenario: --setopt option can be used to set config for specific repo (and repo id may contain dots)
   When I execute dnf with args "repoquery --setopt=setopt.ext.excludepkgs=*"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        wget-0:1.0-1.fc29.src
        wget-0:1.0-1.fc29.x86_64
        """


Scenario: --setopt option can be used with globs to set config for multiple repos
   When I execute dnf with args "repoquery --setopt=setopt*.excludepkgs=wget"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        flac-0:1.0-1.fc29.src
        flac-0:1.0-1.fc29.x86_64
        flac-libs-0:1.0-1.fc29.x86_64
        """
