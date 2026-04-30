``--offline``
    | Store the transaction to be performed offline. See :ref:`offline command <offline_command_ref-label>`, :manpage:`dnf5-offline(8)`.

.. _transient_option-label:

``--transient``
    Applicable only on bootc (bootable containers) systems. Perform transactions using a transient overlay which will be lost on the next reboot. See also the :ref:`persistence <persistence_options-label>` configuration option.

``--store=PATH``
    | Store the current transaction in a directory at the specified ``PATH`` instead of running it.
    | The stored transaction can be performed by the :ref:`replay command <replay_command_ref-label>`, :manpage:`dnf5-replay(8)`.
    | Note that repository ids in the stored transaction are mangled to ``@stored_transaction(repo_id)`` this is required
    | because during replaying the stored repositories are recreated and they might collide with already present repositories
    | (this doesn't apply to the special @System repository).
