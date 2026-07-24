Feature: Accesing a proxy via proxy credentials username and password
         # it just tests whether the proxy credentials are passed correctly
         # URL of the test repo intentionally does not exist and so repoquery
         # is expected to fail
         # dXNlcjoxMjM0NTY= is a base64 representation of user:123456


Scenario: I can store proxy credentials in a repo config
  Given I copy repository "simple-base" for modification
    And I start http server "http_server" at "/{context.dnf.repos[simple-base].path}"
    And I create and substitute file "/etc/yum.repos.d/simple-base.repo" with
        """
        [simple-base]
        name=simple-base test repository
        enabled=1
        baseurl=http://nosuchhost/repo
        proxy=http://localhost:{context.dnf.ports[http_server]}
        proxy_auth_method=basic
        proxy_username=user
        proxy_password=123456
        """
    And I start capturing outbound HTTP requests
   When I execute dnf with args "repoquery abcde"
   Then the exit code is 1
    And every HTTP GET request should match
        | header              | value                  |
        | Proxy-Authorization | Basic dXNlcjoxMjM0NTY= |


Scenario: I can store proxy credentials in dnf.conf
  Given I copy repository "simple-base" for modification
    And I start http server "http_server" at "/{context.dnf.repos[simple-base].path}"
    And I create and substitute file "/etc/yum.repos.d/simple-base.repo" with
        """
        [simple-base]
        name=simple-base test repository
        enabled=1
        baseurl=http://nosuchhost/repo
        """
    And I start capturing outbound HTTP requests
    And I configure dnf with
        | key               | value                                             |
        | proxy             | http://localhost:{context.dnf.ports[http_server]} |
        | proxy_auth_method | basic                                             |
        | proxy_username    | user                                              |
        | proxy_password    | 123456                                            |
   When I execute dnf with args "repoquery abcde"
   Then the exit code is 1
    And every HTTP GET request should match
        | header              | value                  |
        | Proxy-Authorization | Basic dXNlcjoxMjM0NTY= |
