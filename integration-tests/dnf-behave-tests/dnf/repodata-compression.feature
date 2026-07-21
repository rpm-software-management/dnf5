Feature: Repodata compression


@bz1914876
Scenario: Read repodata compressed with zstd
Given I copy repository "simple-base" for modification
  And I use repository "simple-base"
  And I generate repodata for repository "simple-base" with extra arguments "--general-compress-type zstd"
 When I execute dnf with args "repoquery vagare"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      vagare-0:1.0-1.fc29.src
      vagare-0:1.0-1.fc29.x86_64
      """
