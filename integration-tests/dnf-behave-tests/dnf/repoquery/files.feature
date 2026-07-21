Feature: Repoquery tests working with files


Scenario: list files in an rpm including files in filelists.xml
Given I use repository "repoquery-files"
 When I execute dnf with args "repoquery a.x86_64 --files"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      /root-file
      /usr/bin/a-binary
      """


Scenario: list files using --queryformat in an rpm including files in filelists.xml
Given I use repository "repoquery-files"
 When I execute dnf with args "repoquery a.x86_64 --qf %{{files}}"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      /root-file
      /usr/bin/a-binary
      """


Scenario: filter by file in primary.xml
Given I use repository "repoquery-files"
 When I execute dnf with args "repoquery --file /usr/bin/a-binary"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      a-0:1.0-1.fc29.x86_64
      """


Scenario: filter by file in filelists.xml
Given I use repository "repoquery-files"
 When I execute dnf with args "repoquery --file /root-file"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      a-0:1.0-1.fc29.x86_64
      """
