# These tests need to be marked destructive because copr
# doesn't use repositories in installroot, this is tracked as:
# https://github.com/rpm-software-management/dnf5/issues/1497
@destructive
Feature: Test the COPR plugin

Background:
Given I create directory "/{context.dnf.tempdir}/copr"
  And I start http server "copr" at "{context.dnf.tempdir}/copr"
  And I create and substitute file "//etc/dnf/plugins/copr.conf" with
      """
      [main]
      distribution = Fedora
      releasever = 30
      [testhub]
      hostname = localhost
      protocol = http
      port = {context.dnf.ports[copr]}
      """
  And I create and substitute file "/{context.dnf.tempdir}/copr/api_3/rpmrepo/testuser/testproject/Fedora-30/index.html" with
      """
      {{
        "dependencies": [],
        "directories": {{
          "testproject": {{}}
        }},
        "repos": {{
          "Fedora-30": {{
            "arch": {{
              "x86_64": {{
                "opts": {{}}
              }}
            }}
          }},
        }},
        "results_url": "http://project_base_url/"
      }}
      """


Scenario: Test enabling and disabling a project
 When I execute dnf with args "copr enable testhub/testuser/testproject"
 Then the exit code is 0
  And stdout is empty
  And stderr is
      """
      <REPOSYNC>
      Enabling a Copr repository. Please note that this repository is not part
      of the main distribution, and quality may vary.

      The Fedora Project does not exercise any power over the contents of
      this repository beyond the rules outlined in the Copr FAQ at
      <https://docs.copr.fedorainfracloud.org/user_documentation.html#what-i-can-build-in-copr>,
      and packages are not held to any quality or security level.

      Please do not file bug reports about these packages in Fedora
      Bugzilla. In case of problems, contact the owner of this repository.
      """
 When I execute dnf with args "--use-host-config copr disable testhub/testuser/testproject"
 Then the exit code is 0
  And stdout is
      """
      Copr repository 'localhost/testuser/testproject' in '/etc/yum.repos.d/_copr:localhost:testuser:testproject.repo' disabled.
      """
  And stderr is empty


Scenario: Test disabling a project that is not enabled
 When I execute dnf with args "copr disable testhub/testuser/testproject"
 Then the exit code is 1
  And stderr is
      """
      Repository 'localhost/testuser/testproject' not found on this system
      """


Scenario: Test enabling a non-existent repo
 When I execute dnf with args "copr enable testhub/testuser/nonexistent"
 Then the exit code is 1
  And stderr is
      """
      <REPOSYNC>
      >>> Status code: 404 for http://localhost:{context.dnf.ports[copr]}/api_3/rpmrepo/testuser/nonexistent/Fedora-30/ (IP: 127.0.0.1) - http://localhost:{context.dnf.ports[copr]}/api_3/rpmrepo/testuser/nonexistent/Fedora-30/
      >>> Status code: 404 for http://localhost:{context.dnf.ports[copr]}/api_3/rpmrepo/testuser/nonexistent/Fedora-30/ (IP: 127.0.0.1) - http://localhost:{context.dnf.ports[copr]}/api_3/rpmrepo/testuser/nonexistent/Fedora-30/
      >>> Status code: 404 for http://localhost:{context.dnf.ports[copr]}/api_3/rpmrepo/testuser/nonexistent/Fedora-30/ (IP: 127.0.0.1) - http://localhost:{context.dnf.ports[copr]}/api_3/rpmrepo/testuser/nonexistent/Fedora-30/
      >>> Status code: 404 for http://localhost:{context.dnf.ports[copr]}/api_3/rpmrepo/testuser/nonexistent/Fedora-30/ (IP: 127.0.0.1) - http://localhost:{context.dnf.ports[copr]}/api_3/rpmrepo/testuser/nonexistent/Fedora-30/
      >>> Status code: 404 for http://localhost:{context.dnf.ports[copr]}/api_3/rpmrepo/testuser/nonexistent/Fedora-30/ (IP: 127.0.0.1)
      Failed to download files
       Librepo error: Status code: 404 for http://localhost:{context.dnf.ports[copr]}/api_3/rpmrepo/testuser/nonexistent/Fedora-30/ (IP: 127.0.0.1)
      """


# Since curl messages change in fedora 41 don't run on earlier fedoras.
@use.with_os=fedora__ge__41
Scenario: Test enabling a repo with invalid copr configuration
Given I create and substitute file "//etc/dnf/plugins/copr.conf" with
      """
      [main]
      distribution = Fedora
      releasever = 30
      [testhub]
      hostname = localhost
      protocol = http
      # hopefully nothing is ever listening on this port
      port = 2
      """
 When I execute dnf with args "copr enable testhub/testuser/testproject"
 Then the exit code is 1
  And stderr matches line by line
      """
      <REPOSYNC>
      >>> Curl error \(7\): Could not connect to server for http://localhost:2/api_3/rpmrepo/testuser/testproject/Fedora-30/ \[Failed to connect to localhost(:| port )2 after 0 ms: Could not connect to server\] - http://localhost:2/api_3/rpmrepo/testuser/testproject/Fedora-30/
      >>> Curl error \(7\): Could not connect to server for http://localhost:2/api_3/rpmrepo/testuser/testproject/Fedora-30/ \[Failed to connect to localhost(:| port )2 after 0 ms: Could not connect to server\] - http://localhost:2/api_3/rpmrepo/testuser/testproject/Fedora-30/
      >>> Curl error \(7\): Could not connect to server for http://localhost:2/api_3/rpmrepo/testuser/testproject/Fedora-30/ \[Failed to connect to localhost(:| port )2 after 0 ms: Could not connect to server\] - http://localhost:2/api_3/rpmrepo/testuser/testproject/Fedora-30/
      >>> Curl error \(7\): Could not connect to server for http://localhost:2/api_3/rpmrepo/testuser/testproject/Fedora-30/ \[Failed to connect to localhost(:| port )2 after 0 ms: Could not connect to server\] - http://localhost:2/api_3/rpmrepo/testuser/testproject/Fedora-30/
      >>> Curl error \(7\): Could not connect to server for http://localhost:2/api_3/rpmrepo/testuser/testproject/Fedora-30/ \[Failed to connect to localhost(:| port )2 after 0 ms: Could not connect to server\]
      Failed to download files
       Librepo error: Curl error \(7\): Could not connect to server for http://localhost:2/api_3/rpmrepo/testuser/testproject/Fedora-30/ \[Failed to connect to localhost(:| port )2 after 0 ms: Could not connect to server\]
      """


Scenario: Test enabling a repo without any builds for the distribution
Given I create and substitute file "//etc/dnf/plugins/copr.conf" with
      """
      [main]
      distribution = Fedora
      releasever = 31
      [testhub]
      hostname = localhost
      protocol = http
      port = {context.dnf.ports[copr]}
      """
 When I execute dnf with args "copr enable testhub/testuser/testproject"
 Then the exit code is 1
  And stderr is
      """
      <REPOSYNC>
      >>> Status code: 404 for http://localhost:{context.dnf.ports[copr]}/api_3/rpmrepo/testuser/testproject/Fedora-31/ (IP: 127.0.0.1) - http://localhost:{context.dnf.ports[copr]}/api_3/rpmrepo/testuser/testproject/Fedora-31/
      >>> Status code: 404 for http://localhost:{context.dnf.ports[copr]}/api_3/rpmrepo/testuser/testproject/Fedora-31/ (IP: 127.0.0.1) - http://localhost:{context.dnf.ports[copr]}/api_3/rpmrepo/testuser/testproject/Fedora-31/
      >>> Status code: 404 for http://localhost:{context.dnf.ports[copr]}/api_3/rpmrepo/testuser/testproject/Fedora-31/ (IP: 127.0.0.1) - http://localhost:{context.dnf.ports[copr]}/api_3/rpmrepo/testuser/testproject/Fedora-31/
      >>> Status code: 404 for http://localhost:{context.dnf.ports[copr]}/api_3/rpmrepo/testuser/testproject/Fedora-31/ (IP: 127.0.0.1) - http://localhost:{context.dnf.ports[copr]}/api_3/rpmrepo/testuser/testproject/Fedora-31/
      >>> Status code: 404 for http://localhost:{context.dnf.ports[copr]}/api_3/rpmrepo/testuser/testproject/Fedora-31/ (IP: 127.0.0.1)
      Failed to download files
       Librepo error: Status code: 404 for http://localhost:{context.dnf.ports[copr]}/api_3/rpmrepo/testuser/testproject/Fedora-31/ (IP: 127.0.0.1)
      """


Scenario: Test enabling and disabling a repository with dependencies
Given I create and substitute file "/{context.dnf.tempdir}/copr/api_3/rpmrepo/testuser/depproject/Fedora-30/index.html" with
      """
      {{
      "dependencies": [
        {{
            "data": {{
                "owner": "@copr",
                "projectname": "copr-dev"
            }},
            "opts": {{
                "id": "coprdep:localhost:testuser:dep1",
                "name": "Copr copr.fedorainfracloud.org/testuser/ping runtime dependency #1 - @copr/copr-dev"
            }},
            "type": "copr"
        }},
        {{
            "data": {{
            "pattern": "https://download.copr.fedorainfracloud.org/results/@copr/copr-devl/$chroot/"
            }},
            "opts": {{
                "id": "coprdep:some-external-repo",
                "name": "Copr copr.fedorainfracloud.org/testuser/ping runtime dependency #2 - https_download_copr_fedorainfracloud_org_result_copr_copr_dev_chroot"
            }},
            "type": "external_baseurl"
        }}
      ],
        "directories": {{
            "depproject": {{
                "ping:custom:11": {{}}
            }}
        }},
        "repos": {{
          "Fedora-30": {{
            "arch": {{
              "x86_64": {{
                "delete_after_days": 68,
                "opts": {{}}
              }}
            }}
          }},
        }},
        "results_url": "http://repo_base_url"
      }}
      """
 When I execute dnf with args "copr enable testhub/testuser/depproject"
 Then the exit code is 0
  And stdout is empty
  And stderr is
      """
      <REPOSYNC>
      Enabling a Copr repository. Please note that this repository is not part
      of the main distribution, and quality may vary.

      The Fedora Project does not exercise any power over the contents of
      this repository beyond the rules outlined in the Copr FAQ at
      <https://docs.copr.fedorainfracloud.org/user_documentation.html#what-i-can-build-in-copr>,
      and packages are not held to any quality or security level.

      Please do not file bug reports about these packages in Fedora
      Bugzilla. In case of problems, contact the owner of this repository.

      Maintainer of the enabled Copr repository decided to make
      it dependent on other repositories. Such repositories are
      usually necessary for successful installation of RPMs from
      the main Copr repository (they provide runtime dependencies).

      Be aware that the note about quality and bug-reporting
      above applies here too, Fedora Project doesn't control the
      content. Please review the list:

        1. [coprdep:localhost:testuser:dep1]
           baseurl=http://repo_base_url/@copr/copr-dev/Fedora-30-$basearch/

        2. [coprdep:some-external-repo]
           baseurl=https://download.copr.fedorainfracloud.org/results/@copr/copr-devl/Fedora-30-$basearch/

      These repositories are being enabled together with the main
      repository.
      """
 When I execute dnf with args "copr disable testhub/testuser/depproject --use-host-config"
 Then the exit code is 0
  And stdout is
      """
      Copr repository 'localhost/testuser/depproject' in '/etc/yum.repos.d/_copr:localhost:testuser:depproject.repo' disabled.
      """
  And stderr is empty
