@xfail
# The plugin is missing https://github.com/rpm-software-management/dnf5/issues/930
Feature: dnf groups-manager command errors


Scenario: groups-manager reports xml parsing errors
   When I execute dnf with args "groups-manager --load={context.dnf.fixturesdir}/data/groups-manager/comps_syntax_error.xml"
   Then the exit code is 1
    And stderr is
    """
    ERROR: <pkagelist> found with no suitable parent at line:8 col:4
    ERROR: <packagereq> found with no suitable parent at line:9 col:6
    ERROR: Parser error at line:10 col:6
        mismatched tag
    ERROR: Parser error at line:19 col:6
        mismatched tag
    Error: Can't load file "{context.dnf.fixturesdir}/data/groups-manager/comps_syntax_error.xml": Fatal parser error
    """


Scenario: attempt to read non-existent file
  Given file "not_a_file.xml" does not exist
   When I execute dnf with args "groups-manager --load=not_a_file.xml"
   Then the exit code is 1
    And stderr is
    """
    Error: Can't load file "not_a_file.xml": Cannot open not_a_file.xml for reading
    """


Scenario: attempt to write to non-existent directory
  Given file "nonexistent/path" does not exist
   When I execute dnf with args "groups-manager --save=nonexistent/path/out.xml"
   Then the exit code is 1
    And stderr is
    """
    I/O error : No such file or directory
    I/O error : No such file or directory
    Error: Can't save file "nonexistent/path/out.xml": Can't write file nonexistent/path/out.xml
    """


Scenario: reading invalid gz file
  Given I create file "/{context.dnf.tempdir}/invalid.xml.gz" with
    """
    This is not valid gz file
    """
   When I execute dnf with args "groups-manager --load=/{context.dnf.tempdir}/invalid.xml.gz"
   Then the exit code is 1
    And stderr is
    """
    Error: Can't load file "/{context.dnf.tempdir}/invalid.xml.gz": Not a gzipped file (b'Th')
    """


Scenario: cannot remove package from non-existent group
   When I execute dnf with args "groups-manager --load={context.dnf.fixturesdir}/data/groups-manager/comps_a.xml --id not_exists --remove gnomo"
   Then the exit code is 1
    And stderr is
    """
    Error: Can't remove packages from non-existent group
    """


Scenario: cannot edit group without specifying id or name
   When I execute dnf with args "groups-manager --load={context.dnf.fixturesdir}/data/groups-manager/comps_a.xml --description=description"
   Then the exit code is 1
    And stderr is
    """
    Error: Can't edit group without specifying it (use --id or --name)
    """
