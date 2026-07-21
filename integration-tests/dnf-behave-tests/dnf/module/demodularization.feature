# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
# Demodularization is not implemented
# https://github.com/rpm-software-management/dnf5/issues/1852
@xfail
Feature: Modular filtering must ignore demodularized rpms

Background:
  Given I use repository "dnf-ci-fedora-modular"
    And I use repository "dnf-ci-fedora"

@bz1805260
Scenario: Ensuring visibility of non modular rpm in presence of demodularization rpm name in latest module
   When I execute dnf with args "module enable nodejs:5"
   Then the exit code is 0
    And modules state is following
        | Module    | State     | Stream    | Profiles  |
        | nodejs    | enabled   | 5         |           |
   When I execute dnf with args "repoquery npm"
   Then the exit code is 0
    And stdout is
        """
        npm-1:5.3.1-1.module_2011+41787af0.x86_64
        """
   # Repository contains the latest module with demodularized `npm`
   Given I use repository "dnf-ci-fedora-modular-demodularized"
   When I execute dnf with args "repoquery npm"
   Then the exit code is 0
    And stdout is
        """
        npm-1:5.12.1-1.fc29.x86_64
        npm-1:5.3.1-1.module_2011+41787af0.x86_64
        """

@bz1805260
Scenario: Test module info contains Demodularized rpms when defined
   # Repository contains the latest module with demodularized `npm`
   Given I use repository "dnf-ci-fedora-modular-demodularized"
   When I execute dnf with args "module info nodejs:5"
   Then the exit code is 0
    And stderr is
      """
      <REPOSYNC>
      """
    And stdout is
      """
      Name               : nodejs
      Stream             : 5
      Version            : 20150811143429
      Context            : 6c81f848
      Architecture       : x86_64
      Profiles           : default, development, minimal
      Default profiles   :
      Repo               : dnf-ci-fedora-modular-demodularized
      Summary            : Javascript runtime module with quite a long
                         : summary that contains an empty line.
      Description        : Node.js is a platform built on Chrome''s JavaScript runtime for easily building fast, scalable network applications. Node.js uses an event-driven, non-blocking I/O model that makes it lightweight and efficient, perfect for data-intensive real-time applications that run across distributed devices.
      Requires           : platform:[f29]
      Demodularized rpms : npm
      Artifacts          : nodejs-1:5.3.1-1.module_2011+41787af0.src
                         : nodejs-1:5.3.1-1.module_2011+41787af0.x86_64
                         : nodejs-devel-1:5.3.1-1.module_2011+41787af0.x86_64
                         : nodejs-docs-1:5.3.1-1.module_2011+41787af0.noarch

      Name             : nodejs
      Stream           : 5
      Version          : 20150811143428
      Context          : 6c81f848
      Architecture     : x86_64
      Profiles         : default, development, minimal
      Default profiles :
      Repo             : dnf-ci-fedora-modular
      Summary          : Javascript runtime module with quite a long
                       : summary that contains an empty line.
      Description      : Node.js is a platform built on Chrome''s JavaScript runtime for easily building fast, scalable network applications. Node.js uses an event-driven, non-blocking I/O model that makes it lightweight and efficient, perfect for data-intensive real-time applications that run across distributed devices.
      Requires         : platform:[f29]
      Artifacts        : nodejs-1:5.3.1-1.module_2011+41787af0.src
                       : nodejs-1:5.3.1-1.module_2011+41787af0.x86_64
                       : nodejs-devel-1:5.3.1-1.module_2011+41787af0.x86_64
                       : nodejs-docs-1:5.3.1-1.module_2011+41787af0.noarch
                       : npm-1:5.3.1-1.module_2011+41787af0.x86_64

      Hint: [d]efault, [e]nabled, [x]disabled, [i]nstalled, [a]ctive
      """
