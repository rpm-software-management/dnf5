Feature: Test for dnf info


Background: Enable dnf-ci-fedora repository
Given I use repository "dnf-ci-fedora"


Scenario: dnf info nonexistentpkg
 When I execute dnf with args "info non-existent-pkg"
 Then the exit code is 1
  And stderr is
      """
      <REPOSYNC>
      No matching packages to list
      """


Scenario: info all packages available
 When I execute dnf with args "info --available"
 Then the exit code is 0
 Then stdout contains "Name\s+: setup"
 Then stdout contains "Name\s+: basesystem"
 Then stdout contains "Name\s+: glibc"
 Then stdout contains "Name\s+: glibc-common"
 Then stdout contains "Name\s+: glibc-all-langpacks"


Scenario: dnf info --extras (installed pkgs, not from known repos)
 When I execute dnf with args "install setup"
 Then the exit code is 0
Given I drop repository "dnf-ci-fedora"
  And I execute dnf with args "info --extras"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout matches line by line
      """
      Extra packages
      Name            : setup
      Epoch           : 0
      Version         : 2.12.1
      Release         : 1.fc29
      Architecture    : noarch
      Installed size  : 0.0   B
      Source          : setup-2.12.1-1.fc29.src.rpm
      From repository : dnf-ci-fedora
      Summary         : A set of system configuration and setup files
      URL             : https://pagure.io/setup/
      License         : Public Domain
      Description     : The setup package contains a set of important system configuration and
                      : setup files, such as passwd, group, and profile.
      Vendor          : <NULL>
      """


Scenario: dnf info setup (when setup is installed)
 When I execute dnf with args "install setup"
 Then the exit code is 0
Given I drop repository "dnf-ci-fedora"
 When I execute dnf with args "info setup"
 Then stderr is
      """
      <REPOSYNC>
      """
  And stdout matches line by line
      """
      Installed packages
      Name            : setup
      Epoch           : 0
      Version         : 2.12.1
      Release         : 1.fc29
      Architecture    : noarch
      Installed size  : 0.0   B
      Source          : setup-2.12.1-1.fc29.src.rpm
      From repository : dnf-ci-fedora
      Summary         : A set of system configuration and setup files
      URL             : https://pagure.io/setup/
      License         : Public Domain
      Description     : The setup package contains a set of important system configuration and
                      : setup files, such as passwd, group, and profile.
      Vendor          : <NULL>
      """


Scenario: dnf info is case insensitive
 When I execute dnf with args "install setup"
 Then the exit code is 0
Given I drop repository "dnf-ci-fedora"
 When I execute dnf with args "info SETUP"
 Then stderr is
      """
      <REPOSYNC>
      """
  And stdout matches line by line
      """
      Installed packages
      Name            : setup
      Epoch           : 0
      Version         : 2.12.1
      Release         : 1.fc29
      Architecture    : noarch
      Installed size  : 0.0   B
      Source          : setup-2.12.1-1.fc29.src.rpm
      From repository : dnf-ci-fedora
      Summary         : A set of system configuration and setup files
      URL             : https://pagure.io/setup/
      License         : Public Domain
      Description     : The setup package contains a set of important system configuration and
                      : setup files, such as passwd, group, and profile.
      Vendor          : <NULL>
      """


Scenario: dnf info setup (when setup is not installed but it is available)
 When I execute dnf with args "info setup"
 Then stderr is
      """
      <REPOSYNC>
      """
  And stdout matches line by line
      """
      Available packages
      Name           : setup
      Epoch          : 0
      Version        : 2.12.1
      Release        : 1.fc29
      Architecture   : noarch
      Download size  : [0-9.]+ KiB
      Installed size : 0.0   B
      Source         : setup-2.12.1-1.fc29.src.rpm
      Repository     : dnf-ci-fedora
      Summary        : A set of system configuration and setup files
      URL            : https://pagure.io/setup/
      License        : Public Domain
      Description    : The setup package contains a set of important system configuration and
                     : setup files, such as passwd, group, and profile.
      Vendor         :

      Name           : setup
      Epoch          : 0
      Version        : 2.12.1
      Release        : 1.fc29
      Architecture   : src
      Download size  : [0-9.]+ KiB
      Installed size : 616.0   B
      Repository     : dnf-ci-fedora
      Summary        : A set of system configuration and setup files
      URL            : https://pagure.io/setup/
      License        : Public Domain
      Description    : The setup package contains a set of important system configuration and
                     : setup files, such as passwd, group, and profile.
      Vendor         :
      """


Scenario: dnf info --installed setup (when setup is installed)
 When I execute dnf with args "install setup"
 Then the exit code is 0
Given I drop repository "dnf-ci-fedora"
 When I execute dnf with args "info --installed setup"
 Then stderr is
      """
      <REPOSYNC>
      """
  And stdout is
      """
      Installed packages
      Name            : setup
      Epoch           : 0
      Version         : 2.12.1
      Release         : 1.fc29
      Architecture    : noarch
      Installed size  : 0.0   B
      Source          : setup-2.12.1-1.fc29.src.rpm
      From repository : dnf-ci-fedora
      Summary         : A set of system configuration and setup files
      URL             : https://pagure.io/setup/
      License         : Public Domain
      Description     : The setup package contains a set of important system configuration and
                      : setup files, such as passwd, group, and profile.
      Vendor          : <NULL>
      """


Scenario: info --installed alias packages from all enabled repositories
 When I execute dnf with args "install glibc"
 Then the exit code is 0
 When I execute dnf with args "info --installed"
 Then the exit code is 0
 Then stdout contains "Installed packages"
 Then stdout contains "Name\s+: basesystem"
 Then stdout contains "Name\s+: filesystem"
 Then stdout contains "Name\s+: glibc"
 Then stdout contains "Name\s+: glibc-all-langpacks"
 Then stdout contains "Name\s+: glibc-common"
 Then stdout contains "Name\s+: setup"


Scenario: dnf info --available setup (when setup is available)
 When I execute dnf with args "info --available setup"
 Then stderr is
      """
      <REPOSYNC>
      """
  And stdout matches line by line
      """
      Available packages
      Name           : setup
      Epoch          : 0
      Version        : 2.12.1
      Release        : 1.fc29
      Architecture   : noarch
      Download size  : [0-9.]+ KiB
      Installed size : 0.0   B
      Source         : setup-2.12.1-1.fc29.src.rpm
      Repository     : dnf-ci-fedora
      Summary        : A set of system configuration and setup files
      URL            : https://pagure.io/setup/
      License        : Public Domain
      Description    : The setup package contains a set of important system configuration and
                     : setup files, such as passwd, group, and profile.
      Vendor         :

      Name           : setup
      Epoch          : 0
      Version        : 2.12.1
      Release        : 1.fc29
      Architecture   : src
      Download size  : [0-9.]+ KiB
      Installed size : 616.0   B
      Repository     : dnf-ci-fedora
      Summary        : A set of system configuration and setup files
      URL            : https://pagure.io/setup/
      License        : Public Domain
      Description    : The setup package contains a set of important system configuration and
                     : setup files, such as passwd, group, and profile.
      Vendor         :
      """


Scenario: dnf info setup basesystem (when basesystem is not installed)
 When I execute dnf with args "install setup"
 Then the exit code is 0
 When I execute dnf with args "info setup basesystem"
 Then stderr is
      """
      <REPOSYNC>
      """
  And stdout matches line by line
      """
      Installed packages
      Name            : setup
      Epoch           : 0
      Version         : 2.12.1
      Release         : 1.fc29
      Architecture    : noarch
      Installed size  : 0.0   B
      Source          : setup-2.12.1-1.fc29.src.rpm
      From repository : dnf-ci-fedora
      Summary         : A set of system configuration and setup files
      URL             : https://pagure.io/setup/
      License         : Public Domain
      Description     : The setup package contains a set of important system configuration and
                      : setup files, such as passwd, group, and profile.
      Vendor          : <NULL>

      Available packages
      Name           : basesystem
      Epoch          : 0
      Version        : 11
      Release        : 6.fc29
      Architecture   : noarch
      Download size  : [0-9.]+ KiB
      Installed size : 0.0   B
      Source         : basesystem-11-6.fc29.src.rpm
      Repository     : dnf-ci-fedora
      Summary        : The skeleton package which defines a simple Fedora system
      URL            : None
      License        : Public Domain
      Description    : Basesystem defines the components of a basic Fedora system
                     : \(for example, the package installation order to use during bootstrapping\).
                     : Basesystem should be in every installation of a system, and it
                     : should never be removed.
      Vendor         :

      Name           : basesystem
      Epoch          : 0
      Version        : 11
      Release        : 6.fc29
      Architecture   : src
      Download size  : [0-9.]+ KiB
      Installed size : 583.0   B
      Repository     : dnf-ci-fedora
      Summary        : The skeleton package which defines a simple Fedora system
      URL            : None
      License        : Public Domain
      Description    : Basesystem defines the components of a basic Fedora system
                     : \(for example, the package installation order to use during bootstrapping\).
                     : Basesystem should be in every installation of a system, and it
                     : should never be removed.
      Vendor         :

      Name           : setup
      Epoch          : 0
      Version        : 2.12.1
      Release        : 1.fc29
      Architecture   : src
      Download size  : [0-9.]+ KiB
      Installed size : 616.0   B
      Repository     : dnf-ci-fedora
      Summary        : A set of system configuration and setup files
      URL            : https://pagure.io/setup/
      License        : Public Domain
      Description    : The setup package contains a set of important system configuration and
                     : setup files, such as passwd, group, and profile.
      Vendor         :
      """


Scenario: dnf info installed setup basesystem (when basesystem is not installed)
 When I execute dnf with args "install setup"
 Then the exit code is 0
 When I execute dnf with args "info --installed setup basesystem"
 Then stderr is
      """
      <REPOSYNC>
      """
  And stdout matches line by line
      """
      Installed packages
      Name            : setup
      Epoch           : 0
      Version         : 2.12.1
      Release         : 1.fc29
      Architecture    : noarch
      Installed size  : 0.0   B
      Source          : setup-2.12.1-1.fc29.src.rpm
      From repository : dnf-ci-fedora
      Summary         : A set of system configuration and setup files
      URL             : https://pagure.io/setup/
      License         : Public Domain
      Description     : The setup package contains a set of important system configuration and
                      : setup files, such as passwd, group, and profile.
      Vendor          : <NULL>
      """


# Change in behavior compared to dnf4 - Available section contains all available
# packages, installed versions are not filtered out
Scenario: dnf info available setup basesystem (when basesystem is available)
 When I execute dnf with args "install setup"
 Then the exit code is 0
 When I execute dnf with args "info --available setup basesystem"
 Then stdout contains "Available packages"
 Then stdout contains "Name\s+: basesystem"
 Then stdout contains "Name\s+: setup"


Scenario: dnf info setup basesystem (when both are installed)
 When I execute dnf with args "install setup basesystem"
 Then the exit code is 0
 When I execute dnf with args "info setup basesystem"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout matches line by line
      """
      Installed packages
      Name            : basesystem
      Epoch           : 0
      Version         : 11
      Release         : 6.fc29
      Architecture    : noarch
      Installed size  : 0.0   B
      Source          : basesystem-11-6.fc29.src.rpm
      From repository : dnf-ci-fedora
      Summary         : The skeleton package which defines a simple Fedora system
      URL             : None
      License         : Public Domain
      Description     : Basesystem defines the components of a basic Fedora system
                      : \(for example, the package installation order to use during bootstrapping\).
                      : Basesystem should be in every installation of a system, and it
                      : should never be removed.
      Vendor          : <NULL>

      Name            : setup
      Epoch           : 0
      Version         : 2.12.1
      Release         : 1.fc29
      Architecture    : noarch
      Installed size  : 0.0   B
      Source          : setup-2.12.1-1.fc29.src.rpm
      From repository : dnf-ci-fedora
      Summary         : A set of system configuration and setup files
      URL             : https://pagure.io/setup/
      License         : Public Domain
      Description     : The setup package contains a set of important system configuration and
                      : setup files, such as passwd, group, and profile.
      Vendor          : <NULL>

      Available packages
      Name           : basesystem
      Epoch          : 0
      Version        : 11
      Release        : 6.fc29
      Architecture   : src
      Download size  : [0-9.]+ KiB
      Installed size : 583.0   B
      Repository     : dnf-ci-fedora
      Summary        : The skeleton package which defines a simple Fedora system
      URL            : None
      License        : Public Domain
      Description    : Basesystem defines the components of a basic Fedora system
                     : \(for example, the package installation order to use during bootstrapping\).
                     : Basesystem should be in every installation of a system, and it
                     : should never be removed.
      Vendor         :

      Name           : setup
      Epoch          : 0
      Version        : 2.12.1
      Release        : 1.fc29
      Architecture   : src
      Download size  : [0-9.]+ KiB
      Installed size : 616.0   B
      Repository     : dnf-ci-fedora
      Summary        : A set of system configuration and setup files
      URL            : https://pagure.io/setup/
      License        : Public Domain
      Description    : The setup package contains a set of important system configuration and
                     : setup files, such as passwd, group, and profile.
      Vendor         :
      """
 When I execute dnf with args "info --installed setup basesystem"
 Then the exit code is 0
 Then stdout contains "Installed packages"
  And stdout contains "Name\s+: basesystem"
  And stdout contains "Name\s+: setup"
 When I execute dnf with args "info --available setup.noarch basesystem.noarch"
 Then the exit code is 0
 Then stdout contains "Available packages"
  And stdout contains "Name\s+: basesystem"
  And stdout contains "Name\s+: setup"


Scenario Outline: dnf info <upgrades alias>
 When I execute dnf with args "install glibc"
 Then the exit code is 0
Given I use repository "dnf-ci-fedora-updates"
 When I execute dnf with args "info <upgrades alias>"
 Then the exit code is 0
 Then stdout contains "Available upgrades"
  And stdout contains "Name\s+: glibc"
  And stdout contains "Name\s+: glibc-all-langpacks"
  And stdout contains "Name\s+: glibc-common"

Examples:
        | upgrades alias     |
        | --upgrades         |
        | --updates          |


Scenario: dnf info upgrades glibc (when glibc is not installed)
Given I use repository "dnf-ci-fedora-updates"
 When I execute dnf with args "info --upgrades glibc"
 Then the exit code is 1
  And stderr is
      """
      <REPOSYNC>
      No matching packages to list
      """
  And stdout is empty


Scenario: dnf info --obsoletes
 When I execute dnf with args "install glibc"
 Then the exit code is 0
Given I use repository "dnf-ci-fedora-updates"
 When I execute dnf with args "info --obsoletes"
 Then the exit code is 0
  And stderr is
      """
      <REPOSYNC>
      """
  And stdout matches line by line
      """
      Obsoleting packages
      Name           : glibc
      Epoch          : 0
      Version        : 2.28
      Release        : 26.fc29
      Architecture   : x86_64
      Obsoletes      : glibc-0:2.28-9.fc29.x86_64
      Download size  : [0-9.]+ KiB
      Installed size : 0.0   B
      Source         : glibc-2.28-26.fc29.src.rpm
      Repository     : dnf-ci-fedora-updates
      Summary        : The GNU libc libraries
      URL            : http://www.gnu.org/software/glibc/
      License        : LGPLv2\+ and LGPLv2\+ with exceptions and GPLv2\+ and GPLv2\+ with exceptions and BSD and Inner-Net and ISC and Public Domain and GFDL
      Description    : The glibc package contains standard libraries which are used by
                     : multiple programs on the system. In order to save disk space and
                     : memory, as well as to make upgrading easier, common system code is
                     : kept in one place and shared between programs. This particular package
                     : contains the most important sets of shared libraries: the standard C
                     : library and the standard math library. Without these two libraries, a
                     : Linux system will not function.
      Vendor         :
      """


Scenario: dnf info obsoletes setup (when setup is not obsoleted)
 When I execute dnf with args "install setup"
 Then the exit code is 0
 When I execute dnf with args "info --obsoletes setup"
 Then the exit code is 1
  And stderr is
      """
      <REPOSYNC>
      No matching packages to list
      """


@bz1800342
Scenario: dnf info respects repo priorities
  Given I use repository "dnf-ci-fedora-updates" with configuration
        | key           | value   |
        # lower priority than default
        | priority      | 100     |
   When I execute dnf with args "info flac.x86_64"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout matches line by line
        """
        Available packages
        Name           : flac
        Epoch          : 0
        Version        : 1.3.2
        Release        : 8.fc29
        Architecture   : x86_64
        Download size  : [0-9.]+ KiB
        Installed size : 0.0   B
        Source         : flac-1.3.2-8.fc29.src.rpm
        Repository     : dnf-ci-fedora
        Summary        : An encoder/decoder for the Free Lossless Audio Codec
        URL            : http://www.xiph.org/flac/
        License        : BSD and GPLv2\+ and GFDL
        Description    : FLAC stands for Free Lossless Audio Codec. Grossly oversimplified, FLAC
                       : is similar to Ogg Vorbis, but lossless. The FLAC project consists of
                       : the stream format, reference encoders and decoders in library form,
                       : flac, a command-line program to encode and decode FLAC files, metaflac,
                       : a command-line metadata editor for FLAC files and input plugins for
                       : various music players.
                       :
                       : This package contains the command-line tools and documentation.
        Vendor         :
        """


Scenario: dnf info --showduplicates info all (even from lower-priority repo)
  Given I use repository "dnf-ci-fedora-updates" with configuration
        | key           | value   |
        # lower priority than default
        | priority      | 100     |
   When I execute dnf with args "info flac.x86_64 --showduplicates"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout matches line by line
        """
        Available packages
        Name           : flac
        Epoch          : 0
        Version        : 1.3.2
        Release        : 8.fc29
        Architecture   : x86_64
        Download size  : [0-9.]+ KiB
        Installed size : 0.0   B
        Source         : flac-1.3.2-8.fc29.src.rpm
        Repository     : dnf-ci-fedora
        Summary        : An encoder/decoder for the Free Lossless Audio Codec
        URL            : http://www.xiph.org/flac/
        License        : BSD and GPLv2\+ and GFDL
        Description    : FLAC stands for Free Lossless Audio Codec. Grossly oversimplified, FLAC
                       : is similar to Ogg Vorbis, but lossless. The FLAC project consists of
                       : the stream format, reference encoders and decoders in library form,
                       : flac, a command-line program to encode and decode FLAC files, metaflac,
                       : a command-line metadata editor for FLAC files and input plugins for
                       : various music players.
                       :
                       : This package contains the command-line tools and documentation.
        Vendor         :

        Name           : flac
        Epoch          : 0
        Version        : 1.3.3
        Release        : 1.fc29
        Architecture   : x86_64
        Download size  : [0-9.]+ KiB
        Installed size : 0.0   B
        Source         : flac-1.3.3-1.fc29.src.rpm
        Repository     : dnf-ci-fedora-updates
        Summary        : An encoder/decoder for the Free Lossless Audio Codec
        URL            : http://www.xiph.org/flac/
        License        : BSD and GPLv2\+ and GFDL
        Description    : FLAC stands for Free Lossless Audio Codec. Grossly oversimplified, FLAC
                       : is similar to Ogg Vorbis, but lossless. The FLAC project consists of
                       : the stream format, reference encoders and decoders in library form,
                       : flac, a command-line program to encode and decode FLAC files, metaflac,
                       : a command-line metadata editor for FLAC files and input plugins for
                       : various music players.
                       :
                       : This package contains the command-line tools and documentation.
        Vendor         :

        Name           : flac
        Epoch          : 0
        Version        : 1.3.3
        Release        : 2.fc29
        Architecture   : x86_64
        Download size  : [0-9.]+ KiB
        Installed size : 0.0   B
        Source         : flac-1.3.3-2.fc29.src.rpm
        Repository     : dnf-ci-fedora-updates
        Summary        : An encoder/decoder for the Free Lossless Audio Codec
        URL            : http://www.xiph.org/flac/
        License        : BSD and GPLv2\+ and GFDL
        Description    : FLAC stands for Free Lossless Audio Codec. Grossly oversimplified, FLAC
                       : is similar to Ogg Vorbis, but lossless. The FLAC project consists of
                       : the stream format, reference encoders and decoders in library form,
                       : flac, a command-line program to encode and decode FLAC files, metaflac,
                       : a command-line metadata editor for FLAC files and input plugins for
                       : various music players.
                       :
                       : This package contains the command-line tools and documentation.
        Vendor         :

        Name           : flac
        Epoch          : 0
        Version        : 1.3.3
        Release        : 3.fc29
        Architecture   : x86_64
        Download size  : [0-9.]+ KiB
        Installed size : 0.0   B
        Source         : flac-1.3.3-3.fc29.src.rpm
        Repository     : dnf-ci-fedora-updates
        Summary        : An encoder/decoder for the Free Lossless Audio Codec
        URL            : http://www.xiph.org/flac/
        License        : BSD and GPLv2\+ and GFDL
        Description    : FLAC stands for Free Lossless Audio Codec. Grossly oversimplified, FLAC
                       : is similar to Ogg Vorbis, but lossless. The FLAC project consists of
                       : the stream format, reference encoders and decoders in library form,
                       : flac, a command-line program to encode and decode FLAC files, metaflac,
                       : a command-line metadata editor for FLAC files and input plugins for
                       : various music players.
                       :
                       : This package contains the command-line tools and documentation.
        Vendor         :
        """


@bz1800342
Scenario: dnf info doesn't show any available packages when there are no upgrades in the highest-priority repo
  Given I use repository "dnf-ci-fedora-updates" with configuration
        | key           | value   |
        # lower priority than default
        | priority      | 100     |
    And I successfully execute dnf with args "install flac-1.3.3-1.fc29"
   When I execute dnf with args "info flac.x86_64"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout matches line by line
        """
        Installed packages
        Name            : flac
        Epoch           : 0
        Version         : 1.3.3
        Release         : 1.fc29
        Architecture    : x86_64
        Installed size  : 0.0   B
        Source          : flac-1.3.3-1.fc29.src.rpm
        From repository : dnf-ci-fedora-updates
        Summary         : An encoder/decoder for the Free Lossless Audio Codec
        URL             : http://www.xiph.org/flac/
        License         : BSD and GPLv2\+ and GFDL
        Description     : FLAC stands for Free Lossless Audio Codec. Grossly oversimplified, FLAC
                        : is similar to Ogg Vorbis, but lossless. The FLAC project consists of
                        : the stream format, reference encoders and decoders in library form,
                        : flac, a command-line program to encode and decode FLAC files, metaflac,
                        : a command-line metadata editor for FLAC files and input plugins for
                        : various music players.
                        :
                        : This package contains the command-line tools and documentation.
        Vendor          : <NULL>
        """


Scenario: dnf info shows available packages when there are upgrades in the highest-priority repo
  Given I use repository "dnf-ci-fedora-updates" with configuration
        | key           | value   |
        # higher priority than default
        | priority      | 1       |
    And I successfully execute dnf with args "install flac-1.3.3-1.fc29"
   When I execute dnf with args "info flac.x86_64"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout matches line by line
        """
        Installed packages
        Name            : flac
        Epoch           : 0
        Version         : 1.3.3
        Release         : 1.fc29
        Architecture    : x86_64
        Installed size  : 0.0   B
        Source          : flac-1.3.3-1.fc29.src.rpm
        From repository : dnf-ci-fedora-updates
        Summary         : An encoder/decoder for the Free Lossless Audio Codec
        URL             : http://www.xiph.org/flac/
        License         : BSD and GPLv2\+ and GFDL
        Description     : FLAC stands for Free Lossless Audio Codec. Grossly oversimplified, FLAC
                        : is similar to Ogg Vorbis, but lossless. The FLAC project consists of
                        : the stream format, reference encoders and decoders in library form,
                        : flac, a command-line program to encode and decode FLAC files, metaflac,
                        : a command-line metadata editor for FLAC files and input plugins for
                        : various music players.
                        :
                        : This package contains the command-line tools and documentation.
        Vendor          : <NULL>

        Available packages
        Name           : flac
        Epoch          : 0
        Version        : 1.3.3
        Release        : 3.fc29
        Architecture   : x86_64
        Download size  : [0-9.]+ KiB
        Installed size : 0.0   B
        Source         : flac-1.3.3-3.fc29.src.rpm
        Repository     : dnf-ci-fedora-updates
        Summary        : An encoder/decoder for the Free Lossless Audio Codec
        URL            : http://www.xiph.org/flac/
        License        : BSD and GPLv2\+ and GFDL
        Description    : FLAC stands for Free Lossless Audio Codec. Grossly oversimplified, FLAC
                       : is similar to Ogg Vorbis, but lossless. The FLAC project consists of
                       : the stream format, reference encoders and decoders in library form,
                       : flac, a command-line program to encode and decode FLAC files, metaflac,
                       : a command-line metadata editor for FLAC files and input plugins for
                       : various music players.
                       :
                       : This package contains the command-line tools and documentation.
        Vendor         :
        """


Scenario: dnf info doesn't show package with same nevra from lower-priority repo
  Given I configure a new repository "dnf-ci-fedora2" with
        | key     | value                                          |
        | baseurl | file://{context.dnf.repos[dnf-ci-fedora].path} |
        # lower priority than default
        | priority      | 100                                      |
   When I execute dnf with args "info flac.x86_64"
   Then the exit code is 0
    And stderr is
        """
        <REPOSYNC>
        """
    And stdout matches line by line
        """
        Available packages
        Name           : flac
        Epoch          : 0
        Version        : 1.3.2
        Release        : 8.fc29
        Architecture   : x86_64
        Download size  : [0-9.]+ KiB
        Installed size : 0.0   B
        Source         : flac-1.3.2-8.fc29.src.rpm
        Repository     : dnf-ci-fedora
        Summary        : An encoder/decoder for the Free Lossless Audio Codec
        URL            : http://www.xiph.org/flac/
        License        : BSD and GPLv2\+ and GFDL
        Description    : FLAC stands for Free Lossless Audio Codec. Grossly oversimplified, FLAC
                       : is similar to Ogg Vorbis, but lossless. The FLAC project consists of
                       : the stream format, reference encoders and decoders in library form,
                       : flac, a command-line program to encode and decode FLAC files, metaflac,
                       : a command-line metadata editor for FLAC files and input plugins for
                       : various music players.
                       :
                       : This package contains the command-line tools and documentation.
        Vendor         :
        """
