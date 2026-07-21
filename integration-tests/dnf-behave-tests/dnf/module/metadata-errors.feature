# Modularity is disabled since RHEL 11
@not.with_os=rhel__ge__11
@xfail
# Reported as https://github.com/rpm-software-management/dnf5/issues/1855
Feature: Error reading modular metadata

Background: Create a fake modules.yaml, gzip it and damage it
Given I copy repository "dnf-ci-fedora" for modification
  And I use repository "dnf-ci-fedora"
  And I execute "echo 'STUFF' > {context.dnf.repos[dnf-ci-fedora].path}/repodata/my-modules.yaml"
  And I execute "gzip {context.dnf.repos[dnf-ci-fedora].path}/repodata/my-modules.yaml"
  # cut off the last 4 bytes to damage the archive
  And I execute "head -c-4 {context.dnf.repos[dnf-ci-fedora].path}/repodata/my-modules.yaml.gz > {context.dnf.repos[dnf-ci-fedora].path}/repodata/my-modules-damaged.yaml.gz"


@bz1771768
Scenario: Invalid gzip data in modules.yaml.gz
# append invalid data to the gzip archive
Given I execute "echo "GARBAGE" >> {context.dnf.repos[dnf-ci-fedora].path}/repodata/my-modules-damaged.yaml.gz"
  # remove the closing tag of repomd.xml so that we can append
  And I execute "sed -i "s|</repomd>||" {context.dnf.repos[dnf-ci-fedora].path}/repodata/repomd.xml"
  # append the record for the above-created modules.yaml metadata file to repomd.xml
  And I execute "echo "<data type=\"modules\"><checksum type=\"sha256\">`sha256sum {context.dnf.repos[dnf-ci-fedora].path}/repodata/my-modules-damaged.yaml.gz | cut -d' ' -f1`</checksum><location href=\"repodata/my-modules-damaged.yaml.gz\"/></data></repomd>" >> {context.dnf.repos[dnf-ci-fedora].path}/repodata/repomd.xml"
 When I execute dnf with args "install abcde"
 Then the exit code is 1
  And stderr matches line by line
      """
      Error: Error while reading file "/tmp/dnf_ci_installroot_\w+/var/cache/dnf/dnf-ci-fedora-[0-9a-f]+/repodata/my-modules-damaged\.yaml\.gz"\. Likely the archive is damaged\.
      """


# Throws a CloseError instead of ReadError, since libsolv only errors out on when closing the file
# will be fixed in libsolv-0.7.12 or higher
@bz1771768
Scenario: The end of modules.yaml.gz cut off
  # remove the closing tag of repomd.xml so that we can append
  And I execute "sed -i "s|</repomd>||" {context.dnf.repos[dnf-ci-fedora].path}/repodata/repomd.xml"
  # append the record for the above-created modules.yaml metadata file to repomd.xml
  And I execute "echo "<data type=\"modules\"><checksum type=\"sha256\">`sha256sum {context.dnf.repos[dnf-ci-fedora].path}/repodata/my-modules-damaged.yaml.gz | cut -d' ' -f1`</checksum><location href=\"repodata/my-modules-damaged.yaml.gz\"/></data></repomd>" >> {context.dnf.repos[dnf-ci-fedora].path}/repodata/repomd.xml"
 When I execute dnf with args "install abcde"
 Then the exit code is 1
  And stderr is
      """
      Error: Error while reading file "/tmp/dnf_ci_installroot_\w+/var/cache/dnf/dnf-ci-fedora-[0-9a-f]+/repodata/my-modules-damaged\.yaml\.gz"\. Likely the archive is damaged\.
      """
