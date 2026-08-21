# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
Feature: libmodulemd document format tests


@bz2004853
@bz2007166
@bz2007167
Scenario: Metadata containing additional uknown field can be read (the field is ignored)
Given I use repository "additional-field-modulemd"
 When I execute dnf with args "module info nodejs"
 Then stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      Name             : nodejs
      Stream           : 5
      Version          : 20150811143428
      Context          : 6c81f848
      Architecture     : x86_64
      Profiles         : default, development, minimal
      Default profiles :
      Repo             : additional-field-modulemd
      Summary          : Javascript runtime
      Description      : Node.js is a platform built on Chrome
      Requires         : platform:[f29]
      Artifacts        : nodejs-1:5.3.1-1.module_2011+41787af0.src
                       : nodejs-1:5.3.1-1.module_2011+41787af0.x86_64
                       : nodejs-devel-1:5.3.1-1.module_2011+41787af0.x86_64
                       : nodejs-docs-1:5.3.1-1.module_2011+41787af0.noarch
                       : npm-1:5.3.1-1.module_2011+41787af0.x86_64

      Hint: [d]efault, [e]nabled, [x]disabled, [i]nstalled, [a]ctive
      """


@bz2004853
@bz2007166
@bz2007167
Scenario: Metadata containing additional uknown field can be used for modular filtering
Given I use repository "additional-field-modulemd"
  And I use repository "dnf-ci-fedora"
  And I execute dnf with args "module enable nodejs:5"
 When I execute dnf with args "repoquery nodejs"
 Then stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      nodejs-1:5.3.1-1.module_2011+41787af0.src
      nodejs-1:5.3.1-1.module_2011+41787af0.x86_64
      """
