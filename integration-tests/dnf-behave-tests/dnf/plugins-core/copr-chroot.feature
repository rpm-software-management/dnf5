# These tests need to be marked destructive because copr
# doesn't use repositories in installroot, this is tracked as:
# https://github.com/rpm-software-management/dnf5/issues/1497
@destructive
Feature: Test the COPR plugin chroot argument


Background:
Given I create directory "/{context.dnf.tempdir}/copr"
  And I start http server "copr" at "{context.dnf.tempdir}/copr"
  And I create and substitute file "//etc/dnf/plugins/copr.conf" with
      """
      [main]
      distribution = Fedora
      releasever = 31
      [testhub]
      hostname = localhost
      protocol = http
      port = {context.dnf.ports[copr]}
      """
  And I create and substitute file "/{context.dnf.tempdir}/copr/api_3/rpmrepo/testuser/testproject/Fedora-31/index.html" with
      """
      {{
        "dependencies": [],
        "directories": {{
          "testproject": {{}}
        }},
        "repos": {{
          "Fedora-32": {{
            "arch": {{
              "x86_64": {{
                "opts": {{
                  "id": "f32"
                  }}
              }}
            }}
          }},
        }},
        "results_url": "http://project_base_url/"
      }}
      """


Scenario: Test enabling a repo which doesn't have current default chroot available
 When I execute dnf with args "copr enable testhub/testuser/testproject"
 Then the exit code is 1
  And stdout is empty
  And stderr is
      """
      <REPOSYNC>
      Chroot not found in the given Copr project (Fedora-31-x86_64).
      You can choose one of the available chroots explicitly:
       Fedora-32-x86_64
      """


Scenario: Test enabling repo with specified missing chroot
 When I execute dnf with args "copr enable testhub/testuser/testproject Fedora-30-x86_64"
 Then the exit code is 1
  And stdout is empty
  And stderr is
      """
      <REPOSYNC>
      Chroot not found in the given Copr project (Fedora-30-x86_64).
      You can choose one of the available chroots explicitly:
       Fedora-32-x86_64
      """


Scenario: Test enabling a repo with manually specified chroot (different to current) uses opts
 When I execute dnf with args "copr enable testhub/testuser/testproject Fedora-32-x86_64"
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
# --use-host-config has to be used because of https://github.com/rpm-software-management/dnf5/issues/1497
 When I execute dnf with args "repo list f32 --use-host-config"
 Then the exit code is 0
  And stdout is
      """
      repo id repo name
      f32     Copr repo for testproject owned by testuser
      """
  And stderr is empty
