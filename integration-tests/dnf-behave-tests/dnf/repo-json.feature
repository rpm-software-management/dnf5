Feature: Repo command with --json option


Background: Using repositories dnf-ci-fedora and dnf-ci-thirdparty-updates
  Given I use repository "dnf-ci-fedora"
    And I use repository "dnf-ci-thirdparty-updates"
    And I use repository "dnf-ci-fedora-updates" with configuration
        | key     | value |
        | enabled | 0     |
    And I use repository "dnf-ci-thirdparty" with configuration
        | key     | value |
        | enabled | 0     |


Scenario: Repo info without arguments --json
   When I execute dnf with args "repo info --json"
   Then the exit code is 0
    And stdout json matches
    """
    [
      {
        "id":"dnf-ci-fedora",
        "name":"dnf-ci-fedora test repository",
        "is_enabled":true,
        "priority":99,
        "cost":1000,
        "type":"available",
        "exclude_pkgs":[],
        "include_pkgs":[],
        "timestamp":"*",
        "metadata_expire":172800,
        "skip_if_unavailable":false,
        "repo_file_path":"\/tmp\/dnf_ci_installroot_*\/etc\/yum.repos.d\/dnf-ci-fedora.repo",
        "base_url":[
          "file:\/\/\/*\/dnf-behave-tests\/fixtures\/repos\/dnf-ci-fedora"
        ],
        "metalink":"",
        "mirrorlist":"",
        "gpg_key":[],
        "repo_gpgcheck":false,
        "pkg_gpgcheck":true,
        "available_pkgs":289,
        "pkgs":289,
        "size":"*",
        "content_tags":[],
        "distro_tags":[],
        "revision":"1550000000",
        "max_timestamp":"*"
      },
      {
        "id":"dnf-ci-thirdparty-updates",
        "name":"dnf-ci-thirdparty-updates test repository",
        "is_enabled":true,
        "priority":99,
        "cost":1000,
        "type":"available",
        "exclude_pkgs":[],
        "include_pkgs":[],
        "timestamp":"*",
        "metadata_expire":172800,
        "skip_if_unavailable":false,
        "repo_file_path":"\/tmp\/dnf_ci_installroot_*\/etc\/yum.repos.d\/dnf-ci-thirdparty-updates.repo",
        "base_url":[
          "file:\/\/\/*\/dnf-behave-tests\/fixtures\/repos\/dnf-ci-thirdparty-updates"
        ],
        "metalink":"",
        "mirrorlist":"",
        "gpg_key":[],
        "repo_gpgcheck":false,
        "pkg_gpgcheck":true,
        "available_pkgs":6,
        "pkgs":6,
        "size":"*",
        "content_tags":[],
        "distro_tags":[],
        "revision":"1550000000",
        "max_timestamp":"*"
        }
    ]
    """


Scenario: Repo list without arguments --json
   When I execute dnf with args "repo list --json"
   Then the exit code is 0
    And stdout json matches
    """
    [
      {
        "id":"dnf-ci-fedora",
        "name":"dnf-ci-fedora test repository",
        "is_enabled":true
      },
      {
        "id":"dnf-ci-thirdparty-updates",
        "name":"dnf-ci-thirdparty-updates test repository",
        "is_enabled":true
      }
    ]
    """


Scenario: Repo list without arguments --json --all
   When I execute dnf with args "repo list --json --all"
   Then the exit code is 0
    And stdout json matches
    """
    [
      {
        "id":"dnf-ci-fedora-updates",
        "name":"dnf-ci-fedora-updates test repository",
        "is_enabled":false
      },
      {
        "id":"dnf-ci-fedora",
        "name":"dnf-ci-fedora test repository",
        "is_enabled":true
      },
      {
        "id":"dnf-ci-thirdparty-updates",
        "name":"dnf-ci-thirdparty-updates test repository",
        "is_enabled":true
      },
      {
        "id":"dnf-ci-thirdparty",
        "name":"dnf-ci-thirdparty test repository",
        "is_enabled":false
      }
    ]
    """


Scenario: Repo info --json doesn't print REPOSYNC to stdout
   When I execute dnf with args "repo info --json --refresh"
   Then the exit code is 0
    And stdout json matches
    """
    [
      {
        "id":"dnf-ci-fedora",
        "name":"dnf-ci-fedora test repository",
        "is_enabled":true,
        "priority":99,
        "cost":1000,
        "type":"available",
        "exclude_pkgs":[],
        "include_pkgs":[],
        "timestamp":"*",
        "metadata_expire":172800,
        "skip_if_unavailable":false,
        "repo_file_path":"\/tmp\/dnf_ci_installroot_*\/etc\/yum.repos.d\/dnf-ci-fedora.repo",
        "base_url":[
          "file:\/\/\/*\/dnf-behave-tests\/fixtures\/repos\/dnf-ci-fedora"
        ],
        "metalink":"",
        "mirrorlist":"",
        "gpg_key":[],
        "repo_gpgcheck":false,
        "pkg_gpgcheck":true,
        "available_pkgs":289,
        "pkgs":289,
        "size":"*",
        "content_tags":[],
        "distro_tags":[],
        "revision":"1550000000",
        "max_timestamp":"*"
      },
      {
        "id":"dnf-ci-thirdparty-updates",
        "name":"dnf-ci-thirdparty-updates test repository",
        "is_enabled":true,
        "priority":99,
        "cost":1000,
        "type":"available",
        "exclude_pkgs":[],
        "include_pkgs":[],
        "timestamp":"*",
        "metadata_expire":172800,
        "skip_if_unavailable":false,
        "repo_file_path":"\/tmp\/dnf_ci_installroot_*\/etc\/yum.repos.d\/dnf-ci-thirdparty-updates.repo",
        "base_url":[
          "file:\/\/\/*\/dnf-behave-tests\/fixtures\/repos\/dnf-ci-thirdparty-updates"
        ],
        "metalink":"",
        "mirrorlist":"",
        "gpg_key":[],
        "repo_gpgcheck":false,
        "pkg_gpgcheck":true,
        "available_pkgs":6,
        "pkgs":6,
        "size":"*",
        "content_tags":[],
        "distro_tags":[],
        "revision":"1550000000",
        "max_timestamp":"*"
        }
    ]
    """
