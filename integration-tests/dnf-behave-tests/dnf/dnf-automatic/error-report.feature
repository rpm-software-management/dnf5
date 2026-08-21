Feature: dnf-automatic reports an error when transaction failed


Background:
Given I use repository "dnf-ci-automatic-update"


# First, install the "test-1.0" package, which should proceed successfully.
# Then, attempt to update to "test-1.1", which contains a broken scriptlet.
# An error should be reported during the installation of the update.
@bz2170093
Scenario: dnf-automatic reports an error when package installation failed
  Given I successfully execute dnf with args "install test-1.0"
   When I execute dnf with args "automatic --installupdates"
   Then the exit code is 1
    And RPMDB Transaction is empty
    And stdout contains "Failed to install upgrades."
    And stdout contains "Error in %pre scriptlet"
    And stdout contains "Transaction failed: Rpm transaction failed."

# https://github.com/rpm-software-management/dnf/issues/1918
# https://issues.redhat.com/browse/RHEL-61882
Scenario: emitters report errors by default
  Given I create and substitute file "/etc/dnf/automatic.conf" with
    """
    [commands]
    download_updates = yes
    apply_updates = yes

    [emitters]
    emit_via = command_email

    [command_email]
    command_format = "echo {{body}} > {context.dnf.tempdir}/dnf_command_email_error"
    """
    And I successfully execute dnf with args "install test-1.0"
    And file "/{context.dnf.tempdir}/dnf_command_email_error" does not exist
   When I execute dnf with args "automatic --installupdates"
   Then the exit code is 1
    And RPMDB Transaction is empty
    And file "/{context.dnf.tempdir}/dnf_command_email_error" contains lines
    """
    Transaction failed: Rpm transaction failed.
    """

@gdb
Scenario: {body} substituted in command_format for command emitter
  Given I create and substitute file "/etc/dnf/automatic.conf" with
    """
    [commands]
    download_updates = yes
    apply_updates = yes

    [emitters]
    emit_via = command

    [command]
    command_format = "echo {{body}} > {context.dnf.tempdir}/dnf_command_error"
    """
    And I successfully execute dnf with args "install test-1.0"
    And file "/{context.dnf.tempdir}/dnf_command_error" does not exist
   When I execute dnf with args "automatic --installupdates"
   Then the exit code is 1
    And RPMDB Transaction is empty
    And file "/{context.dnf.tempdir}/dnf_command_error" contains lines
    """
    Transaction failed: Rpm transaction failed.
    """
