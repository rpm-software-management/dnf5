Feature: Upgrade with conflicts

Scenario: Conflicts are reported even if the transaction would involve removal of protected packages
 Given I use repository "upgrade-conflicts"
   And I successfully execute dnf with args "install diamond-1-1 spade-1-1"
  When I execute dnf with args "upgrade --setopt=protected_packages=diamond"
  Then stderr is
       """
       <REPOSYNC>
       Failed to resolve the transaction:
       Problem 1: The operation would result in broken dependencies for the following protected packages: diamond
       Problem 2: installed package diamond-1-1.x86_64 requires spade = 1-1, but none of the providers can be installed
         - cannot install both spade-2-1.x86_64 from upgrade-conflicts and spade-1-1.x86_64 from @System
         - cannot install both spade-2-1.x86_64 from upgrade-conflicts and spade-1-1.x86_64 from upgrade-conflicts
         - cannot install the best update candidate for package spade-1-1.x86_64
         - cannot install the best update candidate for package diamond-1-1.x86_64
       You can try to add to command line:
         --no-best to not limit the transaction to the best candidates
       """
