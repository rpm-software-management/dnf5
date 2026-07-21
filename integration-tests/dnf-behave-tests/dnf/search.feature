Feature: Search command


Background:
  Given I use repository "dnf-ci-fedora"


Scenario: without keyword
   When I execute dnf with args "search"
   Then the exit code is 2
   And stderr contains "Missing positional argument \"patterns\" for command \"search\""


Scenario: with keyword
   When I execute dnf with args "search setup"
   Then the exit code is 0
   And stdout is
   """
   <REPOSYNC>
   Matched fields: name (exact), summary
    setup.noarch	A set of system configuration and setup files
    setup.src	A set of system configuration and setup files
   """


@bz1742926
Scenario: with installed and available newest package doesn't duplicate results
   When I execute dnf with args "install setup"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | setup-0:2.12.1-1.fc29.noarch          |
   When I execute dnf with args "search setup"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Matched fields: name (exact), summary
         setup.noarch	A set of system configuration and setup files
         setup.src	A set of system configuration and setup files
        """


Scenario: Only one occurence of a package is in the output when more versions are available
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "search flac"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Matched fields: name (exact)
         flac.src	An encoder/decoder for the Free Lossless Audio Codec
         flac.x86_64	An encoder/decoder for the Free Lossless Audio Codec
        Matched fields: name
         flac-libs.x86_64	Libraries for the Free Lossless Audio Codec
        """


Scenario: All versions of a package are in the output when using the --showduplicates option
  Given I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "search flac --showduplicates"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout is
        """
        Matched fields: name (exact)
         flac-0:1.3.2-8.fc29.src	An encoder/decoder for the Free Lossless Audio Codec
         flac-0:1.3.2-8.fc29.x86_64	An encoder/decoder for the Free Lossless Audio Codec
         flac-0:1.3.3-1.fc29.src	An encoder/decoder for the Free Lossless Audio Codec
         flac-0:1.3.3-1.fc29.x86_64	An encoder/decoder for the Free Lossless Audio Codec
         flac-0:1.3.3-2.fc29.src	An encoder/decoder for the Free Lossless Audio Codec
         flac-0:1.3.3-2.fc29.x86_64	An encoder/decoder for the Free Lossless Audio Codec
         flac-0:1.3.3-3.fc29.src	An encoder/decoder for the Free Lossless Audio Codec
         flac-0:1.3.3-3.fc29.x86_64	An encoder/decoder for the Free Lossless Audio Codec
        Matched fields: name
         flac-libs-0:1.3.2-8.fc29.x86_64	Libraries for the Free Lossless Audio Codec
         flac-libs-0:1.3.3-1.fc29.x86_64	Libraries for the Free Lossless Audio Codec
         flac-libs-0:1.3.3-2.fc29.x86_64	Libraries for the Free Lossless Audio Codec
         flac-libs-0:1.3.3-3.fc29.x86_64	Libraries for the Free Lossless Audio Codec
        """
