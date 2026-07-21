# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
# Missing module install command
# https://github.com/rpm-software-management/dnf5/issues/146
@xfail
Feature: On-disk modulemd data are preferred over repodata in case of a conflict

# Notes regarding operation of modulemd-defaults data merging:
# 1. All repodata data for each module are merged.
# 2. All on-disk data for each module are merged.
# 3. The merged on-disk data and merged repodata data are then merged,
#    with on-disk data for any module overriding the repodata data.
#    This means on-disk data can add to or change stream profiles
#    found in repodata, but never delete stream profiles.

Background: Setup local module defaults
  Given I use repository "dnf-ci-fedora-modular"
    And I use repository "dnf-ci-fedora"
    And I create file "/etc/dnf/modules.defaults.d/local_defaults_a.yaml" with
        """
        ---
        document: modulemd-defaults
        version: 1
        data:
          module: nodejs
          stream: 10
          profiles:
            10: [development]
            8: [development]
        ...
        """
    And I create file "/etc/dnf/modules.defaults.d/local_defaults_b.yaml" with
        """
        ---
        document: modulemd-defaults
        version: 1
        data:
          module: nodejs
          stream: 10
          profiles:
            11: [minimal]
        ...
        """

# the repository defaults are:
#   nodejs:8/default
#   ninja:master/default

Scenario: Local system modulemd defaults are merged and override repo defaults
   When I execute dnf with args "module install nodejs"
   Then the exit code is 0
    And modules state is following
        | Module    | State     | Stream    | Profiles      |
        | nodejs    | enabled   | 10        | development   |


Scenario: Local system modulemd defaults are merged and provide profile for additional stream
   When I execute dnf with args "module install nodejs:11"
   Then the exit code is 0
    And modules state is following
        | Module    | State     | Stream    | Profiles      |
        | nodejs    | enabled   | 11        | minimal       |


Scenario: No local system modulemd defaults to override repo defaults
   When I execute dnf with args "module install ninja"
   Then the exit code is 0
    And modules state is following
        | Module    | State     | Stream    | Profiles      |
        | ninja     | enabled   | master    | default       |
