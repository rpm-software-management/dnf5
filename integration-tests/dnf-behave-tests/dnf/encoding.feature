Feature: Test encoding


Scenario: UTF-8 characters in .repo filename
  Given I configure dnf with
        | key      | value      |
        | reposdir | /testrepos |
    And I configure a new repository "testrepo" in "{context.dnf.installroot}/testrepos" with
        | key     | value                                            |
        | baseurl | {context.scenario.repos_location}/dnf-ci-fedora  |
    And I copy file "{context.dnf.installroot}/testrepos/testrepo.repo" to "testrepos/ř.repo"
    And I delete file "/testrepos/testrepo.repo"
   When I execute dnf with args "repo list"
   Then the exit code is 0
    And stdout contains "testrepo\s+testrepo test repository"
    And stderr is empty


@bz1803038
Scenario: non-UTF-8 characters in .repo filename
  Given I configure dnf with
        | key      | value      |
        | reposdir | /testrepos |
    And I configure a new repository "testrepo" in "{context.dnf.installroot}/testrepos" with
        | key     | value                                            |
        | baseurl | {context.scenario.repos_location}/dnf-ci-fedora  |
    And I copy file "{context.dnf.installroot}/testrepos/testrepo.repo" to "testrepos/{context.invalid_utf8_char}.repo"
    And I delete file "/testrepos/testrepo.repo"
   When I execute dnf with args "repo list"
   Then the exit code is 0
    And stdout contains "testrepo\s+testrepo test repository"
    And stderr is empty


Scenario: non-UTF-8 character in pkgspec
  Given I use repository "miscellaneous"
   When I execute dnf with args "install {context.invalid_utf8_char}ummy"
   Then the exit code is 1
    And stdout is empty
    And stderr is
        """
        <REPOSYNC>
        Failed to resolve the transaction:
        No match for argument: {context.invalid_utf8_char}ummy
        You can try to add to command line:
          --skip-unavailable to skip unavailable packages
        """


Scenario: non-UTF-8 character in baseurl
  Given I use repository "miscellaneous"
   When I execute dnf with args "install dummy --repofrompath=testrepo,{context.invalid_utf8_char}"
   Then the exit code is 1
    And stdout is empty
    And stderr matches line by line
        """
        <REPOSYNC>
        Failed to download metadata
         Librepo error: Empty mirrorlist and no basepath specified!
        """


Scenario: non-UTF-8 character in an option
  Given I use repository "miscellaneous"
   When I execute dnf with args "install dummy --config={context.invalid_utf8_char}"
   Then the exit code is 1
    And stdout is empty
    And stderr is
        """
        Configuration file "{context.invalid_utf8_char}" not found
         cannot open file: (2) - No such file or directory [{context.invalid_utf8_char}]
        """


Scenario: non-UTF-8 character in an option when using corresponding locale
  Given I use repository "miscellaneous"
    And I create file "/{context.invalid_utf8_char}" with
        """
        """
    And I set LC_ALL to "en_US.ISO-8859-1"
   When I execute dnf with args "install dummy --config={context.dnf.installroot}/{context.invalid_utf8_char}"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                    |
        | install       | dummy-1:1.0-1.x86_64       |


@not.with_os=rhel__eq__9
@bz1893176
Scenario: non-UTF-8 character in filename in an installed package
  Given I use repository "miscellaneous"
    And I successfully execute dnf with args "install non_utf_filenames"
   When I execute dnf with args "repoquery --list --installed non_utf_filenames"
   Then the exit code is 0
   When I execute dnf with args "remove non_utf_filenames"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | remove        | non_utf_filenames-0:1.0-1.noarch  |
