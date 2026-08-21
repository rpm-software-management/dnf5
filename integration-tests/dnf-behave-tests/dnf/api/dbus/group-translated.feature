@not.with_mode=dnf5
@dnf5daemon
Feature: D-Bus api: group translated names and descriptions

Background: Enable repository with translated groups
Given I use repository "comps-group-translated"


Scenario: Group list returns translated_name and translated_description when lang is specified
 When I execute python libdnf5 dbus api script with group interface
    """
    options = {{
        "attributes": ["groupid", "name", "translated_name", "translated_description"],
        "lang": "pt_BR"
    }}
    groups = iface_group.list(options)
    groups = sorted(groups, key=lambda g: g.get("groupid", ""))
    for group in groups:
        groupid = str(group.get("groupid", ""))
        name = str(group.get("name", ""))
        translated_name = str(group.get("translated_name", ""))
        translated_description = str(group.get("translated_description", ""))
        print("{{}}: name={{}}, translated_name={{}}, translated_description={{}}".format(
            groupid, name, translated_name, translated_description))
    """
 Then the exit code is 0
  And stdout is
    """
    no-translation-group: name=No Translation Group, translated_name=No Translation Group, translated_description=Group without translations
    simple-group: name=Simple Group, translated_name=Grupo Simples, translated_description=Descrição do grupo simples
    test-group: name=Test Group, translated_name=Grupo de Teste, translated_description=Descrição do grupo de teste.
    """


Scenario: Group list returns default name and description when lang is not specified
 When I execute python libdnf5 dbus api script with group interface
    """
    options = {{
        "attributes": ["groupid", "name", "translated_name", "translated_description"]
    }}
    groups = iface_group.list(options)
    for group in groups:
        groupid = str(group.get("groupid", ""))
        name = str(group.get("name", ""))
        translated_name = str(group.get("translated_name", ""))
        translated_description = str(group.get("translated_description", ""))
        print("{{}}: name={{}}, translated_name={{}}, translated_description={{}}".format(
            groupid, name, translated_name, translated_description))
    """
 Then the exit code is 0
  And stdout contains "test-group: name=Test Group, translated_name=Test Group, translated_description=Test Group description."


Scenario: Group list returns translated_name and translated_description when only these attributes are requested
 When I execute python libdnf5 dbus api script with group interface
    """
    options = {{
        "attributes": ["groupid", "translated_name", "translated_description"],
        "lang": "pt_BR"
    }}
    groups = iface_group.list(options)
    for group in groups:
        groupid = str(group.get("groupid", ""))
        translated_name = str(group.get("translated_name", ""))
        translated_description = str(group.get("translated_description", ""))
        print("{{}}: translated_name={{}}, translated_description={{}}".format(
            groupid, translated_name, translated_description))
    """
 Then the exit code is 0
  And stdout contains "test-group: translated_name=Grupo de Teste, translated_description=Descrição do grupo de teste."
