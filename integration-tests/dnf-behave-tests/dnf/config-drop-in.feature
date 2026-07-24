Feature: Test drop-in directories for configuration files


Scenario: Config files from the drop-in directories are loaded
  Given I use repository "dnf-ci-fedora"
    And I create file "/etc/dnf/libdnf5.conf.d/exclude-filesystem.conf" with
        """
        [main]
        exclude=filesystem
        """
    And I create file "/etc/dnf/libdnf5.conf.d/exclude-wget.conf" with
        """
        [main]
        exclude=wget
        """
    And I create file "/usr/share/dnf5/libdnf.conf.d/exclude-lame.conf" with
        """
        [main]
        exclude=lame
        """
    And I create file "/usr/share/dnf5/libdnf.conf.d/exclude-flac.conf" with
        """
        [main]
        exclude=flac
        """
   When I execute dnf with args "install filesystem wget lame flac setup"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        Argument 'filesystem' matches only excluded packages.
        Argument 'wget' matches only excluded packages.
        Argument 'lame' matches only excluded packages.
        Argument 'flac' matches only excluded packages.
        """


Scenario: In case of the same file name, /etc/dnf/libdnf5.conf.d/... masks usr/share/dnf5/libdnf.conf.d/...
  Given I use repository "dnf-ci-fedora"
    And I create file "/usr/share/dnf5/libdnf.conf.d/exclude-dwn.conf" with
        """
        [main]
        exclude=dwm
        """
    And I create file "/etc/dnf/libdnf5.conf.d/test.conf" with
        """
        [main]
        exclude=filesystem
        """
    And I create file "/usr/share/dnf5/libdnf.conf.d/test.conf" with
        """
        [main]
        exclude=wget
        """
   When I execute dnf with args "install dwm filesystem wget"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        Argument 'dwm' matches only excluded packages.
        Argument 'filesystem' matches only excluded packages.
        """
   When I execute dnf with args "install wget"
   Then the exit code is 0


Scenario: The configs are ordered by name
  Given I use repository "dnf-ci-fedora"
    And I create file "/etc/dnf/libdnf5.conf.d/10-exclude-filesystem.conf" with
        """
        [main]
        exclude=filesystem
        """
    And I create file "/usr/share/dnf5/libdnf.conf.d/20-exclude-wget.conf" with
        """
        [main]
        exclude=wget
        """
    And I create file "/usr/share/dnf5/libdnf.conf.d/30-exclude-only-flac.conf" with
        """
        [main]
        exclude=,flac
        """
    And I create file "/etc/dnf/libdnf5.conf.d/40-exclude-lame.conf" with
        """
        [main]
        exclude=lame
        """
   When I execute dnf with args "install filesystem wget flac lame"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        Argument 'flac' matches only excluded packages.
        Argument 'lame' matches only excluded packages.
        """
   When I execute dnf with args "install filesystem wget"
   Then the exit code is 0


Scenario: The /etc/dnf/dnf.conf is loaded last
  Given I use repository "dnf-ci-fedora"
    And I configure dnf with
        | key                | value      |
        | exclude            | ,flac       |
    And I create file "/etc/dnf/libdnf5.conf.d/10-exclude-filesystem.conf" with
        """
        [main]
        exclude=filesystem
        """
    And I create file "/usr/share/dnf5/libdnf.conf.d/20-exclude-wget.conf" with
        """
        [main]
        exclude=wget
        """
   When I execute dnf with args "install filesystem wget flac"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        Argument 'flac' matches only excluded packages.
        """
   When I execute dnf with args "install filesystem wget"
   Then the exit code is 0


Scenario: Fail when explicitly requested config file doesn't exist
  Given I use repository "dnf-ci-fedora"
   When I execute dnf with args "install filesystem --config /etc/dnf/libdnf5.conf.d/test.conf"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Configuration file "/etc/dnf/libdnf5.conf.d/test.conf" not found
         cannot open file: (2) - No such file or directory [/etc/dnf/libdnf5.conf.d/test.conf]
        """
