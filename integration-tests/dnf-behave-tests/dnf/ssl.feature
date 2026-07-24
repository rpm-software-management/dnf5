Feature: SSL related tests


Scenario: Installing a package from https repository
  Given I use repository "dnf-ci-fedora" as https
   When I execute dnf with args "repolist"
   Then the exit code is 0
    And stdout contains "dnf-ci-fedora\s+dnf-ci-fedora"
   When I execute dnf with args "install filesystem"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | filesystem-0:3.9-2.fc29.x86_64        |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch          |


Scenario: Installing a package from https repository with client verification
  Given I require client certificate verification with certificate "certificates/testcerts/client/cert.pem" and key "certificates/testcerts/client/key.pem"
    And I use repository "dnf-ci-fedora" as https
   When I execute dnf with args "repolist"
   Then the exit code is 0
    And stdout is
       """
       repo id       repo name
       dnf-ci-fedora dnf-ci-fedora test repository
       """
   When I execute dnf with args "install filesystem"
   Then the exit code is 0
    And Transaction is following
        | Action        | Package                               |
        | install       | filesystem-0:3.9-2.fc29.x86_64        |
        | install-dep   | setup-0:2.12.1-1.fc29.noarch          |


@bz1605187
@bz1713627
Scenario: Installing a package using untrusted client cert should fail
  Given I require client certificate verification with certificate "certificates/testcerts/client2/cert.pem" and key "certificates/testcerts/client2/key.pem"
    And I use repository "dnf-ci-fedora" as https
   When I execute dnf with args "install filesystem"
   Then the exit code is 1
        # The list of errors sometimes contains additional errors to the one tested below:
        #   - Curl error (55): Failed sending data to the peer for https://localhost:37237/repodata/repomd.xml [SSL_write() returned SYSCALL, errno = 104]
        #   - Curl error (55): Failed sending data to the peer for https://localhost:38661/repodata/repomd.xml [SSL_write() returned SYSCALL, errno = 32]
        #   - Curl error (56): Failure when receiving data from the peer for https://localhost:37237/repodata/repomd.xml [SSL_write() returned SYSCALL, errno = 32]
        #   - Curl error (56): Failure when receiving data from the peer for https://localhost:[0-9]+/repodata/repomd.xml [OpenSSL SSL_read: error:14094418:SSL routines:ssl3_read_bytes:tlsv1 alert unknown ca, errno 0]"
        #
        # It is nondeterministic, quite rare and the cause is unknown, as well
        # as whether it is an issue with the testing framework or a bug in the
        # DNF stack. This most likely happens for all SSL connections, but is
        # mostly hidden, because librepo does four download attempts and this
        # error typically occurs two or three times (when it happens at all).
        #
        # The connection failures will be hard to debug, untill then, we test
        # for stderr containing the expected output and ignore the superfluous
        # errors.
    And stderr contains ">>> Curl error"
    And stderr contains ">>> Usable URL not found"
    And stderr contains "Failed to download metadata \(baseurl: \"https://localhost:{context.dnf.ports[dnf-ci-fedora]}/\"\) for repository \"dnf-ci-fedora\""


@bz1605187
@bz1713627
Scenario: Installing a package using nonexistent client cert should fail
  Given I require client certificate verification with certificate "certificates/testcerts/nonexistent.pem" and key "certificates/testcerts/nonexistent.pem"
    And I use repository "dnf-ci-fedora" as https
   When I execute dnf with args "install filesystem"
   Then the exit code is 1
    And stderr matches line by line
    """
    <REPOSYNC>
    >>> Curl error \(58\)\: Problem with the local SSL certificate.*
    >>> Curl error \(58\)\: Problem with the local SSL certificate.*
    >>> Curl error \(58\)\: Problem with the local SSL certificate.*
    >>> Curl error \(58\)\: Problem with the local SSL certificate.*
    >>> Usable URL not found
    Failed to download metadata \(baseurl: \"https://localhost:[0-9]+/\"\) for repository \"dnf-ci-fedora\"
    """


Scenario: Installation with untrusted repository should fail
        # https repositories use sslcacert certificates/testcerts/ca/cert.pem
  Given I use repository "simple-base" as https
    And I configure repository "simple-base" with
        | key               | value |
        # replace cacert with another
        | sslcacert         | {context.dnf.fixturesdir}/certificates/testcerts/ca2/cert.pem |
   When I execute dnf with args "install labirinto"
   Then the exit code is 1
    And stderr matches line by line
    """
    <REPOSYNC>
    >>> Curl error \(60\): SSL peer certificate or SSH remote key was not OK for https.*
    >>> Curl error \(60\): SSL peer certificate or SSH remote key was not OK for https.*
    >>> Curl error \(60\): SSL peer certificate or SSH remote key was not OK for https.*
    >>> Curl error \(60\): SSL peer certificate or SSH remote key was not OK for https.*
    >>> Usable URL not found
    Failed to download metadata \(baseurl: \"https://localhost:[0-9]+/\"\) for repository \"simple-base\"
    """


Scenario: Untrusted cert can be overriden with sslverify=False
  Given I use repository "simple-base" as https
    And I configure repository "simple-base" with
        | key               | value |
        | sslcacert         | {context.dnf.fixturesdir}/certificates/testcerts/ca2/cert.pem |
        | sslverify         | false |
   When I execute dnf with args "install labirinto"
   Then the exit code is 0
