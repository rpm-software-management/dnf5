Feature: Tests for the manifest plugin


Background:
Given I use repository "dnf-ci-fedora"
  And I successfully execute dnf with args "install basesystem glibc flac"
  And I set working directory to "{context.dnf.tempdir}"


Scenario: Generate new manifest using specs
   When I execute dnf with args "manifest new abcde http-parser"
   Then the exit code is 0
    And file "/{context.dnf.tempdir}/packages.manifest.yaml" matches line by line
    """
    document: rpm-package-manifest
    version: *
    data:
      repositories:
        - id: dnf-ci-fedora
          baseurl: *
      packages:
        noarch:
          - name: abcde
            repo_id: dnf-ci-fedora
            location: noarch/abcde-2.9.2-1.fc29.noarch.rpm
            checksum: sha256:*
            size: *
            evr: 2.9.2-1.fc29
        x86_64:
          - name: flac
            repo_id: dnf-ci-fedora
            location: x86_64/flac-1.3.2-8.fc29.x86_64.rpm
            checksum: sha256:*
            size: *
            evr: 1.3.2-8.fc29
          - name: http-parser
            repo_id: dnf-ci-fedora
            location: x86_64/http-parser-2.4.0-1.fc29.x86_64.rpm
            checksum: sha256:*
            size: *
            evr: 2.4.0-1.fc29
          - name: wget
            repo_id: dnf-ci-fedora
            location: x86_64/wget-1.19.5-5.fc29.x86_64.rpm
            checksum: sha256:*
            size: *
            evr: 1.19.5-5.fc29
    """


# Check that the real checksum of RPM package is the same one as in the manifest file
Scenario: Non-installed package from manifest has matching checksum
  Given I successfully execute dnf with args "manifest new wget --use-system"
    And I successfully execute dnf with args "manifest download"
   When I execute "sha256sum packages.manifest/wget-1.19.5-5.fc29.x86_64.rpm | cut -d' ' -f1 > real-checksum"
    And I execute "yq '.data.packages.x86_64[].checksum' packages.manifest.yaml | cut -d':' -f2 > manifest-checksum"
    And I execute "diff real-checksum manifest-checksum"
   Then the exit code is 0


Scenario: Generate new manifest using specs and system repo
   When I execute dnf with args "manifest new abcde http-parser --use-system"
   Then the exit code is 0
    And file "/{context.dnf.tempdir}/packages.manifest.yaml" matches line by line
    """
    document: rpm-package-manifest
    version: *
    data:
      repositories:
        - id: dnf-ci-fedora
          baseurl: *
      packages:
        noarch:
          - name: abcde
            repo_id: dnf-ci-fedora
            location: noarch/abcde-2.9.2-1.fc29.noarch.rpm
            checksum: sha256:*
            size: *
            evr: 2.9.2-1.fc29
        x86_64:
          - name: http-parser
            repo_id: dnf-ci-fedora
            location: x86_64/http-parser-2.4.0-1.fc29.x86_64.rpm
            checksum: sha256:*
            size: *
            evr: 2.4.0-1.fc29
          - name: wget
            repo_id: dnf-ci-fedora
            location: x86_64/wget-1.19.5-5.fc29.x86_64.rpm
            checksum: sha256:*
            size: *
            evr: 1.19.5-5.fc29
    """


Scenario: Generate new manifest using prototype input file
  Given I copy file "{context.dnf.fixturesdir}/data/manifest/simple.in.yaml" to "/{context.dnf.tempdir}/rpms.in.yaml"
    And I execute "sed -i 's|$FIXTURES_DIR|{context.dnf.fixturesdir}|' rpms.in.yaml"
   When I execute dnf with args "manifest resolve"
   Then the exit code is 0
    And file "/{context.dnf.tempdir}/packages.manifest.yaml" matches line by line
    """
    document: rpm-package-manifest
    version: *
    data:
      repositories:
        - id: dnf-ci-fedora
          baseurl: *
      packages:
        noarch:
          - name: basesystem
            repo_id: dnf-ci-fedora
            location: noarch/basesystem-11-6.fc29.noarch.rpm
            checksum: sha256:*
            size: *
            evr: 11-6.fc29
          - name: setup
            repo_id: dnf-ci-fedora
            location: noarch/setup-2.12.1-1.fc29.noarch.rpm
            checksum: sha256:*
            size: *
            evr: 2.12.1-1.fc29
        x86_64:
          - name: dwm
            repo_id: dnf-ci-fedora
            location: x86_64/dwm-6.1-1.x86_64.rpm
            checksum: sha256:*
            size: *
            evr: 6.1-1
          - name: filesystem
            repo_id: dnf-ci-fedora
            location: x86_64/filesystem-3.9-2.fc29.x86_64.rpm
            checksum: sha256:*
            size: *
            evr: 3.9-2.fc29
          - name: glibc
            repo_id: dnf-ci-fedora
            location: x86_64/glibc-2.28-9.fc29.x86_64.rpm
            checksum: sha256:*
            size: *
            evr: 2.28-9.fc29
          - name: glibc-all-langpacks
            repo_id: dnf-ci-fedora
            location: x86_64/glibc-all-langpacks-2.28-9.fc29.x86_64.rpm
            checksum: sha256:*
            size: *
            evr: 2.28-9.fc29
          - name: glibc-common
            repo_id: dnf-ci-fedora
            location: x86_64/glibc-common-2.28-9.fc29.x86_64.rpm
            checksum: sha256:*
            size: *
            evr: 2.28-9.fc29
          - name: nodejs
            repo_id: dnf-ci-fedora
            location: x86_64/nodejs-5.12.1-1.fc29.x86_64.rpm
            checksum: sha256:*
            size: *
            evr: 1:5.12.1-1.fc29
          - name: npm
            repo_id: dnf-ci-fedora
            location: x86_64/npm-5.12.1-1.fc29.x86_64.rpm
            checksum: sha256:*
            size: *
            evr: 1:5.12.1-1.fc29
          - name: wget
            repo_id: dnf-ci-fedora
            location: x86_64/wget-1.19.5-5.fc29.x86_64.rpm
            checksum: sha256:*
            size: *
            evr: 1.19.5-5.fc29
    """


Scenario: Generate new manifest using prototype input file and host repositories
  Given I copy file "{context.dnf.fixturesdir}/data/manifest/no-repositories.in.yaml" to "/{context.dnf.tempdir}/rpms.in.yaml"
    And I execute "sed -i 's|$FIXTURES_DIR|{context.dnf.fixturesdir}|' rpms.in.yaml"
   When I execute dnf with args "manifest resolve --use-host-repos"
   Then the exit code is 0
    And file "/{context.dnf.tempdir}/packages.manifest.yaml" matches line by line
    """
    document: rpm-package-manifest
    version: *
    data:
      repositories:
        - id: dnf-ci-fedora
          baseurl: *
      packages:
        noarch:
          - name: basesystem
            repo_id: dnf-ci-fedora
            location: noarch/basesystem-11-6.fc29.noarch.rpm
            checksum: sha256:*
            size: *
            evr: 11-6.fc29
          - name: setup
            repo_id: dnf-ci-fedora
            location: noarch/setup-2.12.1-1.fc29.noarch.rpm
            checksum: sha256:*
            size: *
            evr: 2.12.1-1.fc29
        x86_64:
          - name: dwm
            repo_id: dnf-ci-fedora
            location: x86_64/dwm-6.1-1.x86_64.rpm
            checksum: sha256:*
            size: *
            evr: 6.1-1
          - name: filesystem
            repo_id: dnf-ci-fedora
            location: x86_64/filesystem-3.9-2.fc29.x86_64.rpm
            checksum: sha256:*
            size: *
            evr: 3.9-2.fc29
          - name: glibc
            repo_id: dnf-ci-fedora
            location: x86_64/glibc-2.28-9.fc29.x86_64.rpm
            checksum: sha256:*
            size: *
            evr: 2.28-9.fc29
          - name: glibc-all-langpacks
            repo_id: dnf-ci-fedora
            location: x86_64/glibc-all-langpacks-2.28-9.fc29.x86_64.rpm
            checksum: sha256:*
            size: *
            evr: 2.28-9.fc29
          - name: glibc-common
            repo_id: dnf-ci-fedora
            location: x86_64/glibc-common-2.28-9.fc29.x86_64.rpm
            checksum: sha256:*
            size: *
            evr: 2.28-9.fc29
          - name: nodejs
            repo_id: dnf-ci-fedora
            location: x86_64/nodejs-5.12.1-1.fc29.x86_64.rpm
            checksum: sha256:*
            size: *
            evr: 1:5.12.1-1.fc29
          - name: npm
            repo_id: dnf-ci-fedora
            location: x86_64/npm-5.12.1-1.fc29.x86_64.rpm
            checksum: sha256:*
            size: *
            evr: 1:5.12.1-1.fc29
          - name: wget
            repo_id: dnf-ci-fedora
            location: x86_64/wget-1.19.5-5.fc29.x86_64.rpm
            checksum: sha256:*
            size: *
            evr: 1.19.5-5.fc29
    """


Scenario: Generate new manifest using prototype input file and system repo
  Given I copy file "{context.dnf.fixturesdir}/data/manifest/simple.in.yaml" to "/{context.dnf.tempdir}/rpms.in.yaml"
    And I execute "sed -i 's|$FIXTURES_DIR|{context.dnf.fixturesdir}|' rpms.in.yaml"
   When I execute dnf with args "manifest resolve --use-system"
   Then the exit code is 0
    And file "/{context.dnf.tempdir}/packages.manifest.yaml" matches line by line
    """
    document: rpm-package-manifest
    version: *
    data:
      repositories:
        - id: dnf-ci-fedora
          baseurl: *
      packages:
        x86_64:
          - name: dwm
            repo_id: dnf-ci-fedora
            location: x86_64/dwm-6.1-1.x86_64.rpm
            checksum: sha256:*
            size: *
            evr: 6.1-1
          - name: nodejs
            repo_id: dnf-ci-fedora
            location: x86_64/nodejs-5.12.1-1.fc29.x86_64.rpm
            checksum: sha256:*
            size: *
            evr: 1:5.12.1-1.fc29
          - name: npm
            repo_id: dnf-ci-fedora
            location: x86_64/npm-5.12.1-1.fc29.x86_64.rpm
            checksum: sha256:*
            size: *
            evr: 1:5.12.1-1.fc29
          - name: wget
            repo_id: dnf-ci-fedora
            location: x86_64/wget-1.19.5-5.fc29.x86_64.rpm
            checksum: sha256:*
            size: *
            evr: 1.19.5-5.fc29
    """


# This use case involves loading the system repository with the flac package installed.
# The flac package is also specified in the input file, but as a package for reinstallation, so it should be resolved in the manifest.
Scenario: Generate new manifest with packages for reinstallation using prototype input file and system repo
  Given I copy file "{context.dnf.fixturesdir}/data/manifest/reinstall.in.yaml" to "/{context.dnf.tempdir}/rpms.in.yaml"
    And I execute "sed -i 's|$FIXTURES_DIR|{context.dnf.fixturesdir}|' rpms.in.yaml"
   When I execute dnf with args "manifest resolve --use-system"
   Then the exit code is 0
    And file "/{context.dnf.tempdir}/packages.manifest.yaml" matches line by line
    """
    document: rpm-package-manifest
    version: *
    data:
      repositories:
        - id: dnf-ci-fedora
          baseurl: *
      packages:
        x86_64:
          - name: flac
            repo_id: dnf-ci-fedora
            location: x86_64/flac-1.3.2-8.fc29.x86_64.rpm
            checksum: sha256:*
            size: *
            evr: 1.3.2-8.fc29
          - name: wget
            repo_id: dnf-ci-fedora
            location: x86_64/wget-1.19.5-5.fc29.x86_64.rpm
            checksum: sha256:*
            size: *
            evr: 1.19.5-5.fc29
    """


# Try to install the dnf-ci-conflict package which conflicts with dnf-ci-kernel
# Also excluding the dnf-ci-obsolete which is a obsoleter of dnf-ci-kernel
# This ^ is only possible when --allow-erasing is passed
Scenario: Generate new manifest using prototype input file, system repo and allow erasing
  Given I use repository "protect-running-kernel"
    And I successfully execute dnf with args "install dnf-ci-kernel-1.0"
    And I copy file "{context.dnf.fixturesdir}/data/manifest/allowerasing.in.yaml" to "/{context.dnf.tempdir}/rpms.in.yaml"
    And I execute "sed -i 's|$FIXTURES_DIR|{context.dnf.fixturesdir}|' rpms.in.yaml"
   When I execute dnf with args "manifest resolve --use-system --exclude dnf-ci-obsolete"
   Then the exit code is 0
    And file "/{context.dnf.tempdir}/packages.manifest.yaml" matches line by line
    """
    document: rpm-package-manifest
    version: *
    data:
      repositories:
        - id: protect-running-kernel
          baseurl: *
      packages:
        x86_64:
          - name: dnf-ci-conflict
            repo_id: protect-running-kernel
            location: x86_64/dnf-ci-conflict-1.0-1.x86_64.rpm
            checksum: sha256:*
            size: *
            evr: 1.0-1
    """

# The dnf5 manifest plugin does not yet support modularity
@xfail
Scenario: Generate new manifest using prototype input file with a modular package from the enabled stream
  Given I use repository "dnf-ci-fedora-modular"
    And I copy file "{context.dnf.fixturesdir}/data/manifest/moduleenable.in.yaml" to "/{context.dnf.tempdir}/rpms.in.yaml"
    And I execute "sed -i 's|$FIXTURES_DIR|{context.dnf.fixturesdir}|' rpms.in.yaml"
   When I execute dnf with args "manifest resolve --use-system"
   Then the exit code is 0
    And file "/{context.dnf.tempdir}/packages.manifest.yaml" contains lines
    """
    document: rpm-package-manifest
    version: *
    data:
      repositories:
        - id: dnf-ci-fedora-modular
          baseurl: *
      packages:
        x86_64:
          - name: nodejs
            repo_id: dnf-ci-fedora-modular
            location: x86_64/nodejs-10.11.0-1.module_2200*
            checksum: sha256:*
            size: *
            evr: 1:10.11.0-1.module_2200*
            module: nodejs:10
          - name: npm
            repo_id: dnf-ci-fedora-modular
            location: x86_64/npm-10.11.0-1.module_2200*
            checksum: sha256:*
            size: *
            evr: 1:10.11.0-1.module_2200*
            module: nodejs:10
    ...
    ---
    document: modulemd
    version: 2
    data:
      name: nodejs
      stream: "10"
      version: 20180920144631
      context: 6c81f848
      arch: x86_64
      summary: Javascript runtime
      description: >-
        Node.js is a platform built on Chrome''s JavaScript runtime for easily building
        fast, scalable network applications. Node.js uses an event-driven, non-blocking
        I/O model that makes it lightweight and efficient, perfect for data-intensive
        real-time applications that run across distributed devices.
      license:
        module:
        - MIT
        content:
        - MIT
        - MIT and ASL 2.0 and ISC and BSD
        - MIT and BSD and ISC
      dependencies:
      - buildrequires:
          platform: *
        requires:
          platform: *
      references:
        community: http://nodejs.org
        documentation: http://nodejs.org/en/docs
        tracker: https://github.com/nodejs/node/issues
      profiles:
        default:
          rpms:
          - nodejs
          - npm
        development:
          rpms:
          - nodejs
          - nodejs-devel
          - npm
        minimal:
          rpms:
          - nodejs
      api:
        rpms:
        - nodejs
        - nodejs-devel
        - npm
      components:
        rpms:
          libuv:
            rationale: Platform abstraction layer for Node.js
            repository: git*
            cache: https://src.fedoraproject.org/repo/pkgs/libuv
            ref: "1"
          nghttp2:
            rationale: Needed for HTTP2 support
            repository: git*
            cache: https://src.fedoraproject.org/repo/pkgs/nghttp2
            ref: master
          nodejs:
            rationale: Javascript runtime and npm package manager.
            repository: git*
            cache: https://src.fedoraproject.org/repo/pkgs/nodejs
            ref: "10"
            buildorder: 10
      artifacts:
        rpms:
        - nodejs-1:10.11.0-1.module_2200*
        - nodejs-1:10.11.0-1.module_2200*
        - nodejs-devel-1:10.11.0-1.module_2200*
        - nodejs-docs-1:10.11.0-1.module_2200*
        - npm-1:10.11.0-1.module_2200*
    ...
    """


Scenario: Generate new manifest using installed packages
   When I execute dnf with args "manifest new"
   Then the exit code is 0
    And file "/{context.dnf.tempdir}/packages.manifest.yaml" matches line by line
    """
    document: rpm-package-manifest
    version: *
    data:
      repositories:
        - id: dnf-ci-fedora
          baseurl: *
      packages:
        noarch:
          - name: basesystem
            repo_id: dnf-ci-fedora
            location: noarch/basesystem-11-6.fc29.noarch.rpm
            checksum: sha1:*
            size: *
            evr: 11-6.fc29
          - name: setup
            repo_id: dnf-ci-fedora
            location: noarch/setup-2.12.1-1.fc29.noarch.rpm
            checksum: sha1:*
            size: *
            evr: 2.12.1-1.fc29
        x86_64:
          - name: filesystem
            repo_id: dnf-ci-fedora
            location: x86_64/filesystem-3.9-2.fc29.x86_64.rpm
            checksum: sha1:*
            size: *
            evr: 3.9-2.fc29
          - name: flac
            repo_id: dnf-ci-fedora
            location: x86_64/flac-1.3.2-8.fc29.x86_64.rpm
            checksum: sha1:*
            size: *
            evr: 1.3.2-8.fc29
          - name: glibc
            repo_id: dnf-ci-fedora
            location: x86_64/glibc-2.28-9.fc29.x86_64.rpm
            checksum: sha1:*
            size: *
            evr: 2.28-9.fc29
          - name: glibc-all-langpacks
            repo_id: dnf-ci-fedora
            location: x86_64/glibc-all-langpacks-2.28-9.fc29.x86_64.rpm
            checksum: sha1:*
            size: *
            evr: 2.28-9.fc29
          - name: glibc-common
            repo_id: dnf-ci-fedora
            location: x86_64/glibc-common-2.28-9.fc29.x86_64.rpm
            checksum: sha1:*
            size: *
            evr: 2.28-9.fc29
    """


Scenario: Generate new multiarch manifest file
  Given I use repository "security-upgrade-multilib"
    And I successfully execute dnf with args "manifest new B-1-1 --arch=x86_64,i686"
   Then file "/{context.dnf.tempdir}/packages.manifest.yaml" matches line by line
   """
    document: rpm-package-manifest
    version: *
    data:
    repositories:
      - id: security-upgrade-multilib
        baseurl: file:///opt/ci/dnf-behave-tests/fixtures/repos/security-upgrade-multilib
    packages:
      i686:
        - name: B
          repo_id: security-upgrade-multilib
          location: i686/B-1-1.i686.rpm
          checksum: sha256:*
          size: *
          evr: 1-1
      x86_64:
        - name: B
          repo_id: security-upgrade-multilib
          location: x86_64/B-1-1.x86_64.rpm
          checksum: sha256:*
          size: *
          evr: 1-1
    """


Scenario: Generate new manifest files for each arch
  Given I use repository "security-upgrade-multilib"
    And I successfully execute dnf with args "manifest new B-1-1 --arch=x86_64,i686 --per-arch"
   Then file "/{context.dnf.tempdir}/packages.manifest.x86_64.yaml" matches line by line
   """
   document: rpm-package-manifest
   version: *
   data:
   repositories:
     - id: security-upgrade-multilib
       baseurl: file:///opt/ci/dnf-behave-tests/fixtures/repos/security-upgrade-multilib
   packages:
     x86_64:
       - name: B
         repo_id: security-upgrade-multilib
         location: x86_64/B-1-1.x86_64.rpm
         checksum: sha256:*
         size: *
         evr: 1-1
    """
    And file "/{context.dnf.tempdir}/packages.manifest.i686.yaml" matches line by line
    """
    document: rpm-package-manifest
    version: *
    data:
    repositories:
      - id: security-upgrade-multilib
        baseurl: file:///opt/ci/dnf-behave-tests/fixtures/repos/security-upgrade-multilib
    packages:
      i686:
        - name: B
          repo_id: security-upgrade-multilib
          location: i686/B-1-1.i686.rpm
          checksum: sha256:*
          size: *
          evr: 1-1
    """


Scenario: Download packages from the manifest
  Given I successfully execute dnf with args "manifest new abcde http-parser"
   When I execute dnf with args "manifest download"
   Then the exit code is 0
    And file sha256 checksums are following
        | Path                                                                        | sha256                                                                                          |
        | {context.dnf.tempdir}/packages.manifest/abcde-2.9.2-1.fc29.noarch.rpm       | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora/noarch/abcde-2.9.2-1.fc29.noarch.rpm       |
        | {context.dnf.tempdir}/packages.manifest/flac-1.3.2-8.fc29.x86_64.rpm        | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora/x86_64/flac-1.3.2-8.fc29.x86_64.rpm        |
        | {context.dnf.tempdir}/packages.manifest/http-parser-2.4.0-1.fc29.x86_64.rpm | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora/x86_64/http-parser-2.4.0-1.fc29.x86_64.rpm |
        | {context.dnf.tempdir}/packages.manifest/wget-1.19.5-5.fc29.x86_64.rpm       | file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora/x86_64/wget-1.19.5-5.fc29.x86_64.rpm       |


Scenario: Download multiarch packages from multiarch manifest
  Given I use repository "manifest-multiarch"
    And I successfully execute dnf with args "manifest new foo --arch=x86_64,ppc64"
   When I execute dnf with args "manifest download --arch=x86_64,ppc64"
   Then the exit code is 0
    And file sha256 checksums are following
        | Path                                                                        | sha256                                                                                          |
        | {context.dnf.tempdir}/packages.manifest/foo-1.0-1.x86_64.rpm                | file://{context.dnf.fixturesdir}/repos/manifest-multiarch/x86_64/foo-1.0-1.x86_64.rpm           |
        | {context.dnf.tempdir}/packages.manifest/foo-1.0-1.ppc64.rpm                 | file://{context.dnf.fixturesdir}/repos/manifest-multiarch/ppc64/foo-1.0-1.ppc64.rpm             |
        | {context.dnf.tempdir}/packages.manifest/waldo-1.0-1.noarch.rpm              | file://{context.dnf.fixturesdir}/repos/manifest-multiarch/noarch/waldo-1.0-1.noarch.rpm         |


Scenario: Download multiarch packages from multiarch manifest per arch
  Given I use repository "manifest-multiarch"
    And I successfully execute dnf with args "manifest new foo --arch=x86_64,ppc64"
   When I execute dnf with args "manifest download --arch=x86_64,ppc64 --per-arch"
   Then the exit code is 0
    And file sha256 checksums are following
        | Path                                                                        | sha256                                                                                          |
        | {context.dnf.tempdir}/packages.manifest/x86_64/foo-1.0-1.x86_64.rpm         | file://{context.dnf.fixturesdir}/repos/manifest-multiarch/x86_64/foo-1.0-1.x86_64.rpm           |
        | {context.dnf.tempdir}/packages.manifest/ppc64/foo-1.0-1.ppc64.rpm           | file://{context.dnf.fixturesdir}/repos/manifest-multiarch/ppc64/foo-1.0-1.ppc64.rpm             |
        | {context.dnf.tempdir}/packages.manifest/noarch/waldo-1.0-1.noarch.rpm       | file://{context.dnf.fixturesdir}/repos/manifest-multiarch/noarch/waldo-1.0-1.noarch.rpm         |


Scenario: Download multiarch packages from per-arch manifests
  Given I use repository "manifest-multiarch"
    And I successfully execute dnf with args "manifest new foo --arch=x86_64,ppc64 --per-arch"
    And I successfully execute dnf with args "manifest download --arch=x86_64"
    And I successfully execute dnf with args "manifest download --arch=ppc64"
   Then file sha256 checksums are following
        | Path                                                                        | sha256                                                                                          |
        | {context.dnf.tempdir}/packages.manifest.x86_64/foo-1.0-1.x86_64.rpm         | file://{context.dnf.fixturesdir}/repos/manifest-multiarch/x86_64/foo-1.0-1.x86_64.rpm           |
        | {context.dnf.tempdir}/packages.manifest.x86_64/waldo-1.0-1.noarch.rpm       | file://{context.dnf.fixturesdir}/repos/manifest-multiarch/noarch/waldo-1.0-1.noarch.rpm         |
        | {context.dnf.tempdir}/packages.manifest.ppc64/foo-1.0-1.ppc64.rpm           | file://{context.dnf.fixturesdir}/repos/manifest-multiarch/ppc64/foo-1.0-1.ppc64.rpm             |
        | {context.dnf.tempdir}/packages.manifest.ppc64/waldo-1.0-1.noarch.rpm        | file://{context.dnf.fixturesdir}/repos/manifest-multiarch/noarch/waldo-1.0-1.noarch.rpm         |


Scenario: Download packages from the manifest when a checksum is incorrect
  Given I create and substitute file "/{context.dnf.tempdir}/packages.manifest.yaml" with
    """
    document: rpm-package-manifest
    version: 0.2.2
    data:
      repositories:
        - id: dnf-ci-fedora
          baseurl: file://{context.dnf.fixturesdir}/repos/dnf-ci-fedora
      packages:
        x86_64:
          - name: http-parser
            repo_id: dnf-ci-fedora
            location: x86_64/http-parser-2.4.0-1.fc29.x86_64.rpm
            checksum: sha256:1111111111111111111111111111111111111111111111111111111111111111
            size: 1
            evr: 2.4.0-1.fc29
    """
   When I execute dnf with args "manifest download"
   Then the exit code is 1
    And stderr contains "No package http-parser-2.4.0-1.fc29.x86_64 with checksum 1111111111111111111111111111111111111111111111111111111111111111 available."


Scenario: Install packages from the manifest
  Given I successfully execute dnf with args "manifest new abcde http-parser"
   When I execute dnf with args "manifest install"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                           |
        | install       | abcde-0:2.9.2-1.fc29.noarch       |
        | install       | http-parser-0:2.4.0-1.fc29.x86_64 |
        | install       | wget-0:1.19.5-5.fc29.x86_64       |
