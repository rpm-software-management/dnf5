Feature: Tests for the repository syncing functionality


Background: Force column width
# Some of the curl errors can be quite long and since they are
# truncated: https://github.com/rpm-software-management/dnf5/issues/1829
# we need to force the width to see them in full.
Given I set environment variable "FORCE_COLUMNS" to "500"


@bz1763663
@bz1679509
@bz1692452
Scenario: The default value of skip_if_unavailable is False
  Given I configure dnf with
        | key      | value      |
        | reposdir | /testrepos |
    And I configure a new repository "testrepo" in "{context.dnf.installroot}/testrepos" with
        | key             | value              |
        | baseurl         | /non/existent/repo |
   When I execute dnf with args "makecache"
   Then the exit code is 1
    And stderr matches line by line
    """
    <REPOSYNC>
    >>> Curl error \(37\): (Couldn't|Could not) read a file:// file for file:///non/existent/repo/repodata/repomd.xml \[(Couldn't|Could not) open file /non/existent/repo/repodata/repomd.xml\] - file:///non/existent/repo/repodata/repomd.xml
    >>> Usable URL not found
    Failed to download metadata \(baseurl: "/non/existent/repo"\) for repository "testrepo": Usable URL not found
    """


@bz1689931
Scenario: There is global skip_if_unavailable option
  Given I configure dnf with
        | key                 | value      |
        | reposdir            | /testrepos |
        | skip_if_unavailable | True       |
    And I configure a new repository "testrepo" in "{context.dnf.installroot}/testrepos" with
        | key             | value              |
        | baseurl         | /non/existent/repo |
   When I execute dnf with args "makecache"
   Then the exit code is 0
    And stdout is
    """
    Metadata cache created.
    """
    And stderr matches line by line
    """
    <REPOSYNC>
    >>> Curl error \(37\): (Couldn't|Could not) read a file:// file for file:///non/existent/repo/repodata/repomd.xml \[(Couldn't|Could not) open file /non/existent/repo/repodata/repomd.xml\] - file:///non/existent/repo/repodata/repomd.xml
    >>> Usable URL not found
    Repositories loaded.
    """


Scenario: Per repo skip_if_unavailable configuration
  Given I configure dnf with
        | key      | value      |
        | reposdir | /testrepos |
    And I configure a new repository "testrepo" in "{context.dnf.installroot}/testrepos" with
        | key                 | value              |
        | baseurl             | /non/existent/repo |
        | skip_if_unavailable | True               |
   When I execute dnf with args "makecache"
   Then the exit code is 0
    And stdout is
    """
    Metadata cache created.
    """
    And stderr matches line by line
    """
    <REPOSYNC>
    >>> Curl error \(37\): (Couldn't|Could not) read a file:// file for file:///non/existent/repo/repodata/repomd.xml \[(Couldn't|Could not) open file /non/existent/repo/repodata/repomd.xml\] - file:///non/existent/repo/repodata/repomd.xml
    >>> Usable URL not found
    Repositories loaded.
    """


@bz1689931
Scenario: The repo configuration takes precedence over the global one
  Given I configure dnf with
        | key                 | value      |
        | reposdir            | /testrepos |
        | skip_if_unavailable | True       |
    And I configure a new repository "testrepo" in "{context.dnf.installroot}/testrepos" with
        | key                 | value              |
        | baseurl             | /non/existent/repo |
        | skip_if_unavailable | False              |
   When I execute dnf with args "makecache"
   Then the exit code is 1
    And stderr matches line by line
    """
    <REPOSYNC>
    >>> Curl error \(37\): (Couldn't|Could not) read a file:// file for file:///non/existent/repo/repodata/repomd.xml \[(Couldn't|Could not) open file /non/existent/repo/repodata/repomd.xml\] - file:///non/existent/repo/repodata/repomd.xml
    >>> Usable URL not found
    Failed to download metadata \(baseurl: "/non/existent/repo"\) for repository "testrepo": Usable URL not found
    """


@bz1741442
@bz1752362
Scenario: Test repo_gpgcheck=1 error if repomd.xml.asc is not present
Given I use repository "dnf-ci-fedora" with configuration
      | key           | value |
      | repo_gpgcheck | 1     |
 When I execute dnf with args "makecache"
 Then the exit code is 1
 And stderr contains ">>> Curl error \(37\): (Couldn't|Could not) read a file:// file for file:///.*/dnf-ci-fedora/repodata/repomd.xml.asc"
  And stderr contains ">>> GPG verification is enabled, but GPG signature is not available. This may be an error or the repository does not support GPG verification:"


@bz1713627
# reported as https://github.com/rpm-software-management/dnf5/issues/2064
@xfail
Scenario: Missing baseurl/metalink/mirrorlist
  Given I configure a new repository "testrepo" with
        | key      | value        |
   When I execute dnf with args "makecache"
   Then the exit code is 1
    And stderr is
        """
        <REPOSYNC>
        Failed to download metadata (baseurl: "") for repository "testrepo"
         No valid source (baseurl, mirrorlist or metalink) found for repository "testrepo"
        """
   When I execute dnf with args "makecache --setopt=*.skip_if_unavailable=1"
   Then the exit code is 0
    And stderr is
        """
        Error: Cannot find a valid baseurl for repo: testrepo
        Ignoring repositories: testrepo
        """


@bz1605117
@bz1713627
# reported as https://github.com/rpm-software-management/dnf5/issues/2065
Scenario: Nonexistent GPG key
  Given I use repository "dnf-ci-fedora" with configuration
        | key             | value                                       |
        | gpgkey          | file:///nonexistentkey                      |
        | repo_gpgcheck   | 1                                           |
   When I execute dnf with args "makecache"
   Then the exit code is 1
    And stderr contains "Curl error \(37\): (Couldn't|Could not) read a file:// file for file:///.*/repos/dnf-ci-fedora/repodata/repomd.xml.asc"
    And stderr contains ">>> GPG verification is enabled, but GPG signature is not available. This may be an error or the repository does not support GPG verification:"
   When I execute dnf with args "makecache --setopt=*.skip_if_unavailable=1"
   Then the exit code is 0
    And stderr contains "Curl error \(37\): (Couldn't|Could not) read a file:// file for file:///.*/repos/dnf-ci-fedora/repodata/repomd.xml.asc"
    And stderr contains ">>> GPG verification is enabled, but GPG signature is not available. This may be an error or the repository does not support GPG verification:"
    # See https://github.com/rpm-software-management/dnf5/issues/2064.
    # And stderr contains "Ignoring repositories: dnf-ci-fedora"


@bz1713627
Scenario: Mirrorlist with invalid mirrors
  Given I create file "/tmp/mirrorlist" with
        """
        file:///nonexistent.repo
        http://127.0.0.1:5000/nonexistent
        """
    And I use repository "dnf-ci-fedora" with configuration
        | key             | value                                       |
        | baseurl         |                                             |
        | mirrorlist      | {context.dnf.installroot}/tmp/mirrorlist    |
        | gpgcheck        | 0                                           |
   When I execute dnf with args "makecache"
   Then the exit code is 1
    And stderr matches line by line
    """
    <REPOSYNC>
    >>> Curl error \(37\): (Couldn't|Could not) read a file:// file for file:///nonexistent.repo/repodata/repomd.xml .*
    >>> Curl error \(7\): (Couldn't|Could not) connect to server for http://127.0.0.1:5000/nonexistent/repodata/repomd.xml .*
    >>> Curl error \(7\): (Couldn't|Could not) connect to server for http://127.0.0.1:5000/nonexistent/repodata/repomd.xml .*
    >>> Curl error \(7\): (Couldn't|Could not) connect to server for http://127.0.0.1:5000/nonexistent/repodata/repomd.xml .*
    >>> Usable URL not found
    Failed to download metadata \(mirrorlist: ".*/tmp/mirrorlist"\) for repository "dnf-ci-fedora": Usable URL not found
    """
   When I execute dnf with args "makecache --setopt=*.skip_if_unavailable=1"
   Then the exit code is 0
    And stderr matches line by line
    """
    <REPOSYNC>
    >>> Curl error \(37\): (Couldn't|Could not) read a file:// file for file:///nonexistent.repo/repodata/repomd.xml .*
    >>> Curl error \(7\): (Couldn't|Could not) connect to server for http://127.0.0.1:5000/nonexistent/repodata/repomd.xml .*
    >>> Curl error \(7\): (Couldn't|Could not) connect to server for http://127.0.0.1:5000/nonexistent/repodata/repomd.xml .*
    >>> Curl error \(7\): (Couldn't|Could not) connect to server for http://127.0.0.1:5000/nonexistent/repodata/repomd.xml .*
    >>> Usable URL not found
    Repositories loaded.
    """


Scenario: Mirrorlist with invalid mirrors and one good mirror
  Given I create and substitute file "/tmp/mirrorlist" with
        """
        file:///nonexistent.repo
        http://127.0.0.1:5000/nonexistent
        file://{context.scenario.repos_location}/dnf-ci-fedora
        """
    And I use repository "dnf-ci-fedora" with configuration
        | key             | value                                       |
        | baseurl         |                                             |
        | mirrorlist      | {context.dnf.installroot}/tmp/mirrorlist    |
        | gpgcheck        | 0                                           |
   When I execute dnf with args "makecache"
   Then the exit code is 0
    And stdout is
    """
    Metadata cache created.
    """
    And stderr matches line by line
    """
    <REPOSYNC>
    >>> Curl error \(37\): (Couldn't|Could not) read a file:// file for file:///nonexistent.repo/repodata/repomd.xml .*
    >>> Curl error \(7\): (Couldn't|Could not) connect to server for http://127.0.0.1:5000/nonexistent/repodata/repomd.xml .*
    >>> Curl error \(37\): (Couldn't|Could not) read a file:// file for file:///nonexistent.repo/repodata/primary.xml.zst .*
    >>> Curl error \(7\): (Couldn't|Could not) connect to server for http://127.0.0.1:5000/nonexistent/repodata/primary.xml.zst .*
    Repositories loaded.
    """


Scenario: working and unavailable repos together with skip_if_unavailable enabled
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-fedora-updates" with configuration
        | key             | value               |
        | baseurl         | /non/existent/repo  |
   When I execute dnf with args "rq dwm.x86_64 --setopt=skip_if_unavailable=1"
   Then the exit code is 0
    And stdout is
    """
    dwm-0:6.1-1.x86_64
    """
    And stderr matches line by line
    """
    <REPOSYNC>
    >>> Curl error \(37\): (Couldn't|Could not) read a file:// file for file:///non/existent/repo/repodata/repomd.xml \[(Couldn't|Could not) open file /non/existent/repo/repodata/repomd.xml\] - file:///non/existent/repo/repodata/repomd.xml
    >>> Usable URL not found
     dnf-ci-fedora test repository .*
    Repositories loaded.
    """


Scenario: working and unavailable repos together with skip_if_unavailable disabled
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-fedora-updates" with configuration
        | key             | value               |
        | baseurl         | /non/existent/repo  |
   When I execute dnf with args "rq dwm.x86_64 --setopt=skip_if_unavailable=0"
   Then the exit code is 1
    And stdout is empty
    And stderr matches line by line
    """
    <REPOSYNC>
    >>> Curl error \(37\): (Couldn't|Could not) read a file:// file for file:///non/existent/repo/repodata/repomd.xml \[(Couldn't|Could not) open file /non/existent/repo/repodata/repomd.xml\] - file:///non/existent/repo/repodata/repomd.xml
    >>> Usable URL not found
     dnf-ci-fedora test repository .*
    Failed to download metadata \(baseurl: "/non/existent/repo"\) for repository "dnf-ci-fedora-updates": Usable URL not found
    """


Scenario: two unavailable repos with skip_if_unavailable enabled
  Given I use repository "dnf-ci-fedora" with configuration
        | key             | value               |
        | baseurl         | /non/existent/repo  |
    And I use repository "dnf-ci-fedora-updates" with configuration
        | key             | value               |
        | baseurl         | /non/existent/repo  |
   When I execute dnf with args "rq dwm.x86_64 --setopt=skip_if_unavailable=1"
   Then the exit code is 0
    And stdout is empty
    And stderr contains lines matching
    """
    Updating and loading repositories:
       dnf-ci-fedora-updates test repository .*
    >>> Curl error \(37\): (Couldn't|Could not) read a file:// file for file:///non/existent/repo/repodata/repomd.xml \[(Couldn't|Could not) open file /non/existent/repo/repodata/repomd.xml\] - file:///non/existent/repo/repodata/repomd.xml
    >>> Usable URL not found
       dnf-ci-fedora test repository .*
    >>> Curl error \(37\): (Couldn't|Could not) read a file:// file for file:///non/existent/repo/repodata/repomd.xml \[(Couldn't|Could not) open file /non/existent/repo/repodata/repomd.xml\] - file:///non/existent/repo/repodata/repomd.xml
    >>> Usable URL not found
    Repositories loaded.
    """


Scenario: two unavailable repos with skip_if_unavailable disabled, it attempts to download both repomds but in the end only one error is reported
  Given I use repository "dnf-ci-fedora" with configuration
        | key             | value               |
        | baseurl         | /non/existent/repo  |
    And I use repository "dnf-ci-fedora-updates" with configuration
        | key             | value               |
        | baseurl         | /non/existent/repo  |
   When I execute dnf with args "rq dwm.x86_64 --setopt=skip_if_unavailable=0"
   Then the exit code is 1
    And stdout is empty
    And stderr contains lines matching
    """
    Updating and loading repositories:
       dnf-ci-fedora-updates test repository .*
    >>> Curl error \(37\): (Couldn't|Could not) read a file:// file for file:///non/existent/repo/repodata/repomd.xml \[(Couldn't|Could not) open file /non/existent/repo/repodata/repomd.xml\] - file:///non/existent/repo/repodata/repomd.xml
    >>> Usable URL not found
       dnf-ci-fedora test repository .*
    >>> Curl error \(37\): (Couldn't|Could not) read a file:// file for file:///non/existent/repo/repodata/repomd.xml \[(Couldn't|Could not) open file /non/existent/repo/repodata/repomd.xml\] - file:///non/existent/repo/repodata/repomd.xml
    >>> Usable URL not found
    Failed to download metadata \(baseurl: "/non/existent/repo"\) for repository "dnf-ci-fedora.*": Usable URL not found
    """


Scenario: working and broken repo together with skip_if_unavailable enabled
  Given I use repository "dnf-ci-fedora"
    And I copy repository "dnf-ci-fedora-updates" for modification
    And I delete file "/{context.dnf.repos[dnf-ci-fedora-updates].path}/repodata/primary.xml.zst"
    And I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "rq dwm.x86_64 --setopt=skip_if_unavailable=1"
   Then the exit code is 0
    And stdout is
    """
    dwm-0:6.1-1.x86_64
    """
    And stderr contains lines matching
    """
    Updating and loading repositories:
     dnf-ci-fedora test repository .*
     dnf-ci-fedora-updates test repository .*
    >>> Curl error \(37\): (Couldn't|Could not) read a file:// file for file:///.*/repodata/primary.xml.zst \[(Couldn't|Could not) open file .*/repodata/primary.xml.zst\] - file:///.*/repodata/primary.xml.zst
    >>> No more mirrors to try - All mirrors were already tried without success
    Repositories loaded.
    """


Scenario: working and broken repo together with skip_if_unavailable disabled
  Given I use repository "dnf-ci-fedora"
    And I copy repository "dnf-ci-fedora-updates" for modification
    And I delete file "/{context.dnf.repos[dnf-ci-fedora-updates].path}/repodata/primary.xml.zst"
    And I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "rq dwm.x86_64 --setopt=skip_if_unavailable=0"
   Then the exit code is 1
    And stdout is empty
    And stderr contains lines matching
    """
    Updating and loading repositories:
     dnf-ci-fedora test repository .*
     dnf-ci-fedora-updates test repository .*
    >>> Curl error \(37\): (Couldn't|Could not) read a file:// file for file:///.*/repodata/primary.xml.zst \[(Couldn't|Could not) open file .*/repodata/primary.xml.zst\] - file:///.*/repodata/primary.xml.zst
    >>> No more mirrors to try - All mirrors were already tried without success
    Failed to download metadata \(baseurl: "file:///.*/dnf-ci-fedora.*"\) for repository "dnf-ci-fedora.*": Cannot download, all mirrors were already tried without success
    """


Scenario: two broken repos together with skip_if_unavailable enabled
  Given I copy repository "dnf-ci-fedora" for modification
    And I delete file "/{context.dnf.repos[dnf-ci-fedora].path}/repodata/primary.xml.zst"
    And I use repository "dnf-ci-fedora"
    And I copy repository "dnf-ci-fedora-updates" for modification
    And I delete file "/{context.dnf.repos[dnf-ci-fedora-updates].path}/repodata/primary.xml.zst"
    And I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "rq dwm.x86_64 --setopt=skip_if_unavailable=1"
   Then the exit code is 0
    And stdout is empty
    And stderr contains lines matching
    """
    Updating and loading repositories:
     dnf-ci-fedora-updates test repository .*
    >>> Curl error \(37\): (Couldn't|Could not) read a file:// file for file:///.*/repodata/primary.xml.zst \[(Couldn't|Could not) open file .*/repodata/primary.xml.zst\] - file:///.*/repodata/primary.xml.zst
    >>> No more mirrors to try - All mirrors were already tried without success
     dnf-ci-fedora test repository .*
    >>> Curl error \(37\): (Couldn't|Could not) read a file:// file for file:///.*/repodata/primary.xml.zst \[(Couldn't|Could not) open file .*/repodata/primary.xml.zst\] - file:///.*/repodata/primary.xml.zst
    >>> No more mirrors to try - All mirrors were already tried without success
    Repositories loaded.
    """


Scenario: two broken repos together with skip_if_unavailable enabled
  Given I copy repository "dnf-ci-fedora" for modification
    And I delete file "/{context.dnf.repos[dnf-ci-fedora].path}/repodata/primary.xml.zst"
    And I use repository "dnf-ci-fedora"
    And I copy repository "dnf-ci-fedora-updates" for modification
    And I delete file "/{context.dnf.repos[dnf-ci-fedora-updates].path}/repodata/primary.xml.zst"
    And I use repository "dnf-ci-fedora-updates"
   When I execute dnf with args "rq dwm.x86_64 --setopt=skip_if_unavailable=0"
   Then the exit code is 1
    And stdout is empty
    And stderr contains lines matching
    """
    Updating and loading repositories:
     dnf-ci-fedora-updates test repository .*
    >>> Curl error \(37\): (Couldn't|Could not) read a file:// file for file:///.*/repodata/primary.xml.zst \[(Couldn't|Could not) open file .*/repodata/primary.xml.zst\] - file:///.*/repodata/primary.xml.zst
    >>> No more mirrors to try - All mirrors were already tried without success
     dnf-ci-fedora test repository .*
    >>> Curl error \(37\): (Couldn't|Could not) read a file:// file for file:///.*/repodata/primary.xml.zst \[(Couldn't|Could not) open file .*/repodata/primary.xml.zst\] - file:///.*/repodata/primary.xml.zst
    >>> No more mirrors to try - All mirrors were already tried without success
    Failed to download metadata \(baseurl: "file:///.*/dnf-ci-fedora.*"\) for repository "dnf-ci-fedora.*": Cannot download, all mirrors were already tried without success
    """


Scenario: working repo and repo with missing repomd.xml.asc with skip_if_unavailable enabled
  Given I use repository "dnf-ci-fedora-updates" with configuration
        | key           | value |
        | repo_gpgcheck | 1     |
    And I use repository "dnf-ci-fedora"
   When I execute dnf with args "rq dwm.x86_64 --setopt=skip_if_unavailable=1"
   Then the exit code is 0
    And stdout is
    """
    dwm-0:6.1-1.x86_64
    """
    And stderr contains lines matching
    """
    Updating and loading repositories:
     dnf-ci-fedora-updates test repository .*
    >>> Curl error \(37\): (Couldn't|Could not) read a file:// file for file:///.*/repodata/repomd.xml.asc \[(Couldn't|Could not) open file .*/repodata/repomd.xml.asc\] - file:///.*/repodata/repomd.xml.asc
    >>> GPG verification is enabled, but GPG signature is not available. This may be an error or the repository does not support GPG verification: .*
     dnf-ci-fedora test repository .*
    Repositories loaded.
    """


Scenario: working repo and repo with missing repomd.xml.asc with skip_if_unavailable disabled
  Given I use repository "dnf-ci-fedora-updates" with configuration
        | key           | value |
        | repo_gpgcheck | 1     |
    And I use repository "dnf-ci-fedora"
   When I execute dnf with args "rq dwm.x86_64 --setopt=skip_if_unavailable=0"
   Then the exit code is 1
    And stdout is empty
    And stderr contains lines matching
    """
    Updating and loading repositories:
     dnf-ci-fedora-updates test repository .*
    >>> Curl error \(37\): (Couldn't|Could not) read a file:// file for file:///.*/repodata/repomd.xml.asc \[(Couldn't|Could not) open file .*/repodata/repomd.xml.asc\] - file:///.*/repodata/repomd.xml.asc
    >>> GPG verification is enabled, but GPG signature is not available. This may be an error or the repository does not support GPG verification: .*
     dnf-ci-fedora test repository .*
    Failed to download metadata \(baseurl: "file:///.*/repos/dnf-ci-fedora-updates"\) for repository "dnf-ci-fedora-updates": GPG verification is enabled, but GPG signature is not available. This may be an error or the repository does not support GPG verification: Curl error \(37\): (Couldn't|Could not) read a file:// file for file:///.*/repos/dnf-ci-fedora-updates/repodata/repomd.xml.asc \[(Couldn't|Could not) open file /.*/repos/dnf-ci-fedora-updates/repodata/repomd.xml.asc\]
    """


Scenario Outline: working repo and repo without any mirror or baseurl is an error when skip_if_unavailable is <value>
  Given I use repository "dnf-ci-fedora"
    And I configure a new repository "testrepo" with
        | key     | value |
        | enabled | True  |
   When I execute dnf with args "rq dwm.x86_64 --setopt=skip_if_unavailable=<value>"
   Then the exit code is 1
    And stdout is empty
    And stderr is
    """
    Updating and loading repositories:
    Failed to set up metadata download
     No valid source (baseurl, mirrorlist or metalink) found for repository "testrepo"
    """
Examples:
    | value |
    | 1     |
    | 0     |


Scenario Outline: working repo and repo with invalid baseurl is an error when skip_if_unavailable is <value>
  Given I use repository "dnf-ci-fedora"
    And I configure a new repository "testrepo" with
        | key     | value   |
        | enabled | True    |
        # the baseurl is a local file but doesn't exist
        | baseurl | invalid |
   When I execute dnf with args "rq dwm.x86_64 --setopt=skip_if_unavailable=<value>"
   Then the exit code is 1
    And stdout is empty
    And stderr is
    """
    Updating and loading repositories:
    Failed to download metadata
     Librepo error: Empty mirrorlist and no basepath specified!
    """
Examples:
    | value |
    | 1     |
    | 0     |


Scenario: working repo and repo with absolute path baseurl to a missing file when skip_if_unavailable is enabled
  Given I use repository "dnf-ci-fedora"
    And I configure a new repository "testrepo" with
        | key     | value                 |
        | enabled | True                  |
        # the baseurl is an absolute path but the file is missing
        | baseurl | file:///nfsmount/repo |
   When I execute dnf with args "rq dwm.x86_64 --setopt=skip_if_unavailable=1"
   Then the exit code is 0
    And stdout is
    """
    dwm-0:6.1-1.x86_64
    """
    And stderr contains lines matching
    """
    Updating and loading repositories:
     testrepo test repository .*
    >>> Curl error \(37\): (Couldn't|Could not) read a file:// file for file:///.*/repodata/repomd.xml \[(Couldn't|Could not) open file .*/repodata/repomd.xml\] - file:///.*/repodata/repomd.xml
    >>> Usable URL not found
     dnf-ci-fedora test repository .*
    Repositories loaded.
    """


Scenario: working repo and repo with absolute path baseurl to a missing file when skip_if_unavailable is disabled
  Given I use repository "dnf-ci-fedora"
    And I configure a new repository "testrepo" with
        | key     | value                 |
        | enabled | True                  |
        # the baseurl is an absolute path but the file is missing
        | baseurl | file:///nfsmount/repo |
   When I execute dnf with args "rq dwm.x86_64 --setopt=skip_if_unavailable=0"
   Then the exit code is 1
    And stderr contains lines matching
    """
    Updating and loading repositories:
     testrepo test repository .*
    >>> Curl error \(37\): (Couldn't|Could not) read a file:// file for file:///.*/repodata/repomd.xml \[(Couldn't|Could not) open file .*/repodata/repomd.xml\] - file:///.*/repodata/repomd.xml
    >>> Usable URL not found
     dnf-ci-fedora test repository .*
    Failed to download metadata \(baseurl: "file:///nfsmount/repo"\) for repository "testrepo": Usable URL not found
    """
