@xfail
# Reported as https://github.com/rpm-software-management/ci-dnf-stack/issues/1632
Feature: Plugin API


Background:
Given I do not disable plugins
  And I configure dnf with
      | key        | value                             |
      | pluginpath | {context.dnf.installroot}/plugins |


@bz1650446
Scenario: Plugins have access to transaction items after transaction is finished
Given I create file "/plugins/test.py" with
  """
  import dnf

  class Test(dnf.Plugin):
    name = "testTransactionAccess"
    def __init__(self, base, cli):
        super(Test, self).__init__(base, cli)

    def transaction(self):
        downloads = self.base.transaction.install_set
        for pkg in downloads:
            print("Plugin has access to installed pkg: " +
                   pkg.name + "+" + pkg.version + "+" + pkg.release + "+" + pkg.arch)

  """
  And I use repository "dnf-ci-fedora"
 When I execute dnf with args "install setup filesystem"
 Then stdout contains "Plugin has access to installed pkg: setup\+2.12.1\+1.fc29\+noarch"
 Then stdout contains "Plugin has access to installed pkg: filesystem\+3.9\+2.fc29\+x86_64"


@bz1626093
Scenario: Plugins can edit http headers
Given I create file "/plugins/test.py" with
  """
  import dnf

  class Test(dnf.Plugin):
    name = "testHttpHeader"
    def __init__(self, base, cli):
        super(Test, self).__init__(base, cli)

    def config(self):
        for repoid, repo in self.base.repos.items():
            repo.set_http_headers([
                "custom_user_key20190218: custom_user_value",
            ])
  """
  And I use repository "dnf-ci-fedora" as http
  And I start capturing outbound HTTP requests
 When I execute dnf with args "makecache"
 Then every HTTP GET request should match
    | header                  | value             |
    | custom_user_key20190218 | custom_user_value |
