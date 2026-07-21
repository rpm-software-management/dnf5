Feature: Tests for python_plugins_loader plugin


Scenario: libdnf5 python plugin is loaded and runs
  Given I enable plugin "python_plugins_loader"
    And I configure dnf with
      | key            | value                                             |
      | pluginconfpath | {context.dnf.installroot}/etc/dnf/libdnf5-plugins |
    And I create and substitute file "/etc/dnf/libdnf5-plugins/python_plugins_loader.conf" with
    """
    [main]
    enabled = 1
    """
    And I enable plugin "test_plugin"
    And I create and substitute file "/etc/dnf/libdnf5-plugins/python_plugins_loader.d/test_plugin.conf" with
    """
    [main]
    enabled = 1
    """
    And I set environment variable "LIBDNF_PYTHON_PLUGIN_DIR" to "/{context.dnf.installroot}"
    And I create file "/test_plugin.py" with
    """
    import libdnf5

    class Plugin(libdnf5.plugin.IPlugin):
        def __init__(self, data):
            super(Plugin, self).__init__(data)

        @staticmethod
        def get_api_version():
            return libdnf5.PluginAPIVersion(2, 1)

        @staticmethod
        def get_name():
            return 'test_plugin'

        @staticmethod
        def get_version():
            return libdnf5.plugin.Version(0, 1, 0)

        def repos_configured(self):
            config = self.get_base().get_config()
            print(self.get_name() + ' - skip_if_unavailable = ' +
                  str(config.skip_if_unavailable))
            return True
    """
   When I execute dnf with args "rq --setopt=skip_if_unavailable=false"
   Then the exit code is 0
    And stdout is
    """
    test_plugin - skip_if_unavailable = False
    """
   When I execute dnf with args "rq --setopt=skip_if_unavailable=true"
   Then the exit code is 0
    And stdout is
    """
    test_plugin - skip_if_unavailable = True
    """
