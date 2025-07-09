###############################################
 Modifications to the public API in dnf 5.3.0.0
###############################################

This page lists the differences in the public API of `DNF5 5.3 <https://github.com/rpm-software-management/dnf5/releases/tag/5.3.0.0>`_ compared to the previous major version, DNF5 5.2.

`Here <https://raw.githubusercontent.com/rpm-software-management/dnf5/main/doc/api/changes_dnf5.3.diff>`_'s the full diff showing the changes in the libdnf5 API.

Group and Environment queries return weak pointers
==================================================

Before: Group and Environment queries stored objects directly, this meant that
when a query was destroyed any references to its objects became invalid.

Now: The Groups and Environments are created only once and stored in libdnf5::comps::CompsSack in libdnf5::Base.
Queries contain only weak pointers to objects in CompsSack. When a query is destroyed any
pointers to Groups or Environments obtained from it remain valid (as long as the Base lives).
This means that the queries return pointers instead of references.

.. code-block:: cpp

    comps::GroupQuery group_query(base);
    for (const auto & group : group_query) {
        std::cout << "id: " << group.get_groupid();
    }

has to turn into:

.. code-block:: cpp

    comps::GroupQuery group_query(base);
    for (const auto & group : group_query) {
        std::cout << "id: " << group->get_groupid();
    }
