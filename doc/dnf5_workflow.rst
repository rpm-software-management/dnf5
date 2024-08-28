#############
DNF5 Workflow
#############


Typical DNF5 workflow consists of:

#. set up loggers
#. create base
#. add dnf5 commands
#. load dnf5 plugins: (See :ref:`DNF5 Plugins <dnf5 plugins tutorial>` for details.)
    #. dnf5 plugin ``init`` hook
    #. dnf5 plugin ``create_commands`` hook
    #. run command specific ``set_parent_command`` step
    #. run command specific ``set_argument_parser`` step
    #. | run command specific ``register_subcommands`` step
       | (for native dnf5 commands the last 3 steps are done as a part of 'add dnf5 commands')
#. load aliases
#. parse command line arguments
#. run command specific ``pre_configure`` step
#. load main configuration
#. enable/disable libdnf5 plugins
#. base setup:
    #. load libdnf5 plugins (See :ref:`LIBDNF5 Plugins <libdnf5 plugins tutorial>` for details.)
    #. libdnf5 plugin ``init`` hook
    #. libdnf5 plugin ``pre_base_setup`` hook
    #. lock installroot
    #. load Vars and lock varsdir
    #. libdnf5 plugin ``post_base_setup`` hook
#. create repo sack
#. create repos from system configuration
    * perform Vars substitution on repository id and all values (See :ref:`Repo Variables <repo_variables-label>` for details.)
#. create repos from paths (such as --repofrompath arg)
    * perform Vars substitution on specified id and path
#. apply repository setopts (such as --setopt=fedora.metadata_expire=0)
#. run command specific ``configure`` step
#. libdnf5 plugin ``repos_configured`` hook
#. if command requires privileges check for them
#. load repositories:
    #. if required load system repository
    #. if required load enabled repositories:
        #. load metadata from cache if valid
        #. try to reuse root's cache
        #. metadata download
        #. metadata gpg check
        #. if required import repository gpg keys and try again
#. libdnf5 plugin ``repos_loaded`` hook
#. run command specific ``load_additional_packages`` step
#. run command specific ``run`` step
#. if the command produced a goal:
    #. libdnf5 plugin ``pre_add_cmdline_packages`` hook
    #. add commandline packages
    #. libdnf5 plugin ``post_add_cmdline_packages`` hook
    #. resolve goal (resolve dependencies)
    #. run command specific ``goal_resolved`` step
    #. print transaction table
    #. check for user approval
    #. download inbound transaction packages
    #. check gpg signatures for inbound transaction packages
    #. lock transaction ``libdnf5::utils::Locker``
    #. create rpm transaction
    #. run rpm test transaction
    #. libdnf5 plugin ``pre_transaction`` hook
    #. start database transaction
    #. run rpm transaction
    #. update system_state (See :manpage:`dnf5-system-state(7)`, :ref:`System state <systemstate_misc_ref-label>` for details.)
    #. finish database transaction
    #. libdnf5 plugin ``post_transaction`` hook
    #. unlock transaction ``libdnf5::utils::Locker``
#. libdnf5 plugin ``finish`` hook
#. dnf5 plugin ``finish`` hook
