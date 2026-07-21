# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import print_function

from behave import use_fixture

from steps.fixtures.ftpd import ftpd_fixture
from steps.fixtures.httpd import httpd_fixture


def start_server_based_on_type(context, server_dir, rtype, certs=None):
    if rtype == "ftp":
        use_fixture(ftpd_fixture, context)
        host, port = context.scenario.ftpd.new_ftp_server(server_dir)
    else:
        use_fixture(httpd_fixture, context)

        if rtype == "http":
            host, port = context.scenario.httpd.new_http_server(server_dir)
        elif rtype == "https":
            host, port = context.scenario.httpd.new_https_server(
                server_dir, certs["cacert"], certs["cert"], certs["key"],
                client_verification=bool(context.dnf._get("client_ssl")))
        else:
            raise AssertionError("Unknown server type: %s" % rtype)

    return host, port


def stop_server_type(context, path, rtype):
    if rtype == "ftp":
        context.scenario.ftpd.stop_server(path)
    else:
        context.scenario.httpd.stop_server(path)
