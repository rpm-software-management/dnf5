@xfail
# The plugin is missing https://github.com/rpm-software-management/dnf5/issues/930
Feature: dnf groups-manager command


Scenario Outline: groups-manager can read <filename> file
  Given I copy file "{context.dnf.fixturesdir}/data/groups-manager/comps_a.xml" to "/{context.dnf.tempdir}/comps_a.xml"
    And I compress file "/{context.dnf.tempdir}/comps_a.xml" using "gz"
   When I execute dnf with args "groups-manager --load=/{context.dnf.tempdir}/<filename>"
   Then the exit code is 0
    And stdout is
   """
   <?xml version="1.0" encoding="UTF-8"?>
   <!DOCTYPE comps PUBLIC "-//Red Hat, Inc.//DTD Comps info//EN" "comps.dtd">
   <comps>
     <group>
       <id>group-a</id>
       <name>Group A</name>
       <default>false</default>
       <uservisible>true</uservisible>
       <packagelist>
         <packagereq type="default">gnomo</packagereq>
       </packagelist>
     </group>
     <group>
       <id>group-common</id>
       <name>Common Group A</name>
       <default>false</default>
       <uservisible>true</uservisible>
       <packagelist>
         <packagereq type="default">gnomo</packagereq>
         <packagereq type="default">pazzo</packagereq>
       </packagelist>
     </group>
   </comps>
   """

Examples:
    | filename          |
    | comps_a.xml       |
    | comps_a.xml.gz    |


Scenario Outline: merging two comps files <filename1>, <filename2>
   When I execute dnf with args "groups-manager --load={context.dnf.fixturesdir}/data/groups-manager/<filename1> --load={context.dnf.fixturesdir}/data/groups-manager/<filename2> --save=/{context.dnf.tempdir}/out.xml"
   Then the exit code is 0
    And the files "{context.dnf.fixturesdir}/data/groups-manager/<result>" and "/{context.dnf.tempdir}/out.xml" do not differ

Examples:
    | filename1         | filename2         | result            |
    | comps_a.xml       | comps_b.xml       | comps_a+b.xml     |
    | comps_b.xml       | comps_a.xml       | comps_b+a.xml     |


Scenario: edit group properties
   When I copy file "{context.dnf.fixturesdir}/data/groups-manager/comps_a.xml" to "/{context.dnf.tempdir}/comps_a.xml"
   And I execute dnf with args "groups-manager --merge=/{context.dnf.tempdir}/comps_a.xml --id=group-a --description="new description" --display-order=111 --not-user-visible --translated-name="cs:Skupina A" --translated-name="de:Gruppe A" --translated-description="cs:Překrásný popis skupiny A""
   Then the exit code is 0
    And file "/{context.dnf.tempdir}/comps_a.xml" contents is
    """
    <?xml version="1.0" encoding="UTF-8"?>
    <!DOCTYPE comps PUBLIC "-//Red Hat, Inc.//DTD Comps info//EN" "comps.dtd">
    <comps>
      <group>
        <id>group-a</id>
        <name>Group A</name>
        <name xml:lang="cs">Skupina A</name>
        <name xml:lang="de">Gruppe A</name>
        <description>new description</description>
        <description xml:lang="cs">Překrásný popis skupiny A</description>
        <default>false</default>
        <uservisible>false</uservisible>
        <display_order>111</display_order>
        <packagelist>
          <packagereq type="default">gnomo</packagereq>
        </packagelist>
      </group>
      <group>
        <id>group-common</id>
        <name>Common Group A</name>
        <default>false</default>
        <uservisible>true</uservisible>
        <packagelist>
          <packagereq type="default">gnomo</packagereq>
          <packagereq type="default">pazzo</packagereq>
        </packagelist>
      </group>
    </comps>
    """


Scenario Outline: edit group filelists - add package of type "<type>"
  Given I use repository "simple-base"
   When I execute dnf with args "groups-manager --load={context.dnf.fixturesdir}/data/groups-manager/comps_a.xml --save /{context.dnf.tempdir}/out.xml --id=group-a <group_modifier> vagare"
   Then the exit code is 0
    And file "/{context.dnf.tempdir}/out.xml" matches line by line
    """
    <\?xml version="1\.0" encoding="UTF-8"\?>
    <!DOCTYPE comps PUBLIC "-//Red Hat, Inc\.//DTD Comps info//EN" "comps\.dtd">
    <comps>
      <group>
        <id>group-a</id>
        <name>Group A</name>
        <default>false</default>
        <uservisible>true</uservisible>
        <packagelist>
          <packagereq type="default">gnomo</packagereq>
          <packagereq type="<type>"(?: requires="")?>vagare</packagereq>
        </packagelist>
      </group>
      <group>
        <id>group-common</id>
        <name>Common Group A</name>
        <default>false</default>
        <uservisible>true</uservisible>
        <packagelist>
          <packagereq type="default">gnomo</packagereq>
          <packagereq type="default">pazzo</packagereq>
        </packagelist>
      </group>
    </comps>
    """

Examples:
    | group_modifier    | type      |
    |                   | default   |
    | --mandatory       | mandatory |
    | --optional        | optional  |


@bz2013633
Scenario Outline: Package to add to a group can be specified by "<type>"
  Given I use repository "simple-base"
   When I execute dnf with args "groups-manager --name="New group" --print <spec>"
   Then the exit code is 0
    And stderr is
    """
    <REPOSYNC>
    """
    And stdout is
    """
    <?xml version="1.0" encoding="UTF-8"?>
    <!DOCTYPE comps PUBLIC "-//Red Hat, Inc.//DTD Comps info//EN" "comps.dtd">
    <comps>
      <group>
        <id>newgroup</id>
        <name>New group</name>
        <default>false</default>
        <uservisible>true</uservisible>
        <packagelist>
          <packagereq type="default">vagare</packagereq>
        </packagelist>
      </group>
    </comps>
    """
    And stderr is empty

Examples:
    | type              | spec                          |
    | NEVRA             | vagare-0:1.0-1.fc29.x86_64    |
    | NVR               | vagare-1.0-1.fc29             |
    | NV                | vagare-1.0                    |
    | NA                | vagare.x86_64                 |
    | N                 | vagare                        |


Scenario: edit group filelists - add package with dependencies
  Given I use repository "simple-base"
   When I execute dnf with args "groups-manager --load={context.dnf.fixturesdir}/data/groups-manager/comps_a.xml --save /{context.dnf.tempdir}/out.xml --id=group-a --dependencies vagare"
   Then the exit code is 0
    And file "/{context.dnf.tempdir}/out.xml" matches line by line
    """
    <\?xml version="1\.0" encoding="UTF-8"\?>
    <!DOCTYPE comps PUBLIC "-//Red Hat, Inc\.//DTD Comps info//EN" "comps\.dtd">
    <comps>
      <group>
        <id>group-a</id>
        <name>Group A</name>
        <default>false</default>
        <uservisible>true</uservisible>
        <packagelist>
          <packagereq type="default">gnomo</packagereq>
          <packagereq type="default"(?: requires="")?>labirinto</packagereq>
          <packagereq type="default"(?: requires="")?>vagare</packagereq>
        </packagelist>
      </group>
      <group>
        <id>group-common</id>
        <name>Common Group A</name>
        <default>false</default>
        <uservisible>true</uservisible>
        <packagelist>
          <packagereq type="default">gnomo</packagereq>
          <packagereq type="default">pazzo</packagereq>
        </packagelist>
      </group>
    </comps>
    """


Scenario: add a new group
  Given I use repository "simple-base"
   When I execute dnf with args "groups-manager --save /{context.dnf.tempdir}/out.xml --id=new-group --name="New group" --print vagare"
   Then the exit code is 0
    And file "/{context.dnf.tempdir}/out.xml" matches line by line
    """
    <\?xml version="1\.0" encoding="UTF-8"\?>
    <!DOCTYPE comps PUBLIC "-//Red Hat, Inc\.//DTD Comps info//EN" "comps\.dtd">
    <comps>
      <group>
        <id>new-group</id>
        <name>New group</name>
        <default>false</default>
        <uservisible>true</uservisible>
        <packagelist>
          <packagereq type="default"(?: requires="")?>vagare</packagereq>
        </packagelist>
      </group>
    </comps>
    """


Scenario: groups-manager --print is working
   When I execute dnf with args "groups-manager --save /{context.dnf.tempdir}/out.xml --id=new-group --name="New group" --print"
   Then the exit code is 0
    And file "/{context.dnf.tempdir}/out.xml" contents is
    """
    <?xml version="1.0" encoding="UTF-8"?>
    <!DOCTYPE comps PUBLIC "-//Red Hat, Inc.//DTD Comps info//EN" "comps.dtd">
    <comps>
      <group>
        <id>new-group</id>
        <name>New group</name>
        <default>false</default>
        <uservisible>true</uservisible>
      </group>
    </comps>
    """
    And stderr is
    """
    <REPOSYNC>
    """
    And stdout is
    """
    <?xml version="1.0" encoding="UTF-8"?>
    <!DOCTYPE comps PUBLIC "-//Red Hat, Inc.//DTD Comps info//EN" "comps.dtd">
    <comps>
      <group>
        <id>new-group</id>
        <name>New group</name>
        <default>false</default>
        <uservisible>true</uservisible>
      </group>
    </comps>
    """
