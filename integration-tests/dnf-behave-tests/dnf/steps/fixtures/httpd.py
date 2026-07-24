# -*- coding: utf-8 -*-

from __future__ import absolute_import
from steps.fixtures.server import ServerContext
from __future__ import print_function

from behave import fixture
import multiprocessing
import os
import ssl
import sys

PY3 = sys.version_info.major >= 3
if PY3:
    from http.server import SimpleHTTPRequestHandler
    from socketserver import TCPServer
else:
    from SimpleHTTPServer import SimpleHTTPRequestHandler
    from SocketServer import TCPServer


class AccessRecord(object):
    """Represents an HTTP request processed by a server instance."""

    def __init__(self, handler):
        self.command = handler.command
        self.path = handler.path
        self.headers = handler.headers

    def __str__(self):
        headers = ['%s: %s' % (k, v) for k, v in self.headers.items()]
        return '\n%s %s\n%s' % (self.command, self.path, '\n'.join(headers))


class NoLogHttpHandler(SimpleHTTPRequestHandler):
    def log_request(self, *args, **kwargs):
        pass


class HttpHandler(SimpleHTTPRequestHandler):
    def log_request(self, *args, **kwargs):
        if not self.server._conf.get('logging', False):
            return
        self.server._log.append(AccessRecord(self))

    def do_GET(self):
        # Respond with the specific status code if configured, otherwise just
        # process the request as usual.
        if 'status' in self.server._conf:
            self.send_response(self.server._conf['status'])
            if self.path not in self.server._conf["response_headers"]:
                self.end_headers()
                return
        if self.path in self.server._conf["response_headers"]:
            for header in self.server._conf["response_headers"][self.path]:
                self.send_header(header[0], header[1])
            self.end_headers()
            return
        super(HttpHandler, self).do_GET()


class HttpServerContext(ServerContext):
    """
    This object manages group of simple http servers. Each of them is run in
    separate process and serves configured directory.

    Usage:
    ctx = HttpServerContext()
    ctx.new_http_server('/path/to/directory/supposed/to/be/served')
    ctx.new_http_server('/path/to/other_directory/supposed/to/be/served')
    do_stuff()
    ctx.shutdown()
    """

    @staticmethod
    def http_server(address, path, error, log, conf):
        try:
            os.chdir(path)
            httpd = TCPServer(address, HttpHandler)
            httpd._log = log
            httpd._conf = conf
            httpd.serve_forever()
        except Exception as e:
            error.value = str(e)

    @staticmethod
    def https_server(address, path, error, cacert, cert, key, client_verification=False):
        try:
            os.chdir(path)
            httpd = TCPServer(address, NoLogHttpHandler)
            context = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH, cafile=cacert)
            context.load_cert_chain(certfile=cert, keyfile=key)
            if client_verification:
                context.verify_mode = ssl.CERT_REQUIRED
            httpd.socket = context.wrap_socket(httpd.socket, server_side=True)
            httpd.serve_forever()
        except Exception as e:
            error.value = str(e)

    def __init__(self):
        super(HttpServerContext, self).__init__()
        # list of AccessRecord objects
        self._log = multiprocessing.Manager().list()
        self._conf = multiprocessing.Manager().dict()
        self._conf["response_headers"] = dict()

    def new_http_server(self, path):
        return self._start_server(path, self.http_server, self._log, self._conf)

    def new_https_server(self, path, cacert, cert, key, client_verification):
        return self._start_server(
            path, self.https_server, cacert, cert, key, client_verification)

    @property
    def log(self):
        return self._log

    def clear_log(self):
        """
        Empty the log of the http server
        """
        del self.log[:]

    @property
    def conf(self):
        return self._conf


if __name__ == '__main__':
    ctx = HttpServerContext()
    certpath = '../../../fixtures/certificates/testcerts'
    cacert = os.path.realpath(os.path.join(certpath, 'ca/cert.pem'))
    host, port = ctx.new_https_server(
        '../../../fixtures/repos/',
        cacert,
        os.path.realpath(os.path.join(certpath, 'server/cert.pem')),
        os.path.realpath(os.path.join(certpath, 'server/key.pem')),
        False)
    curl = 'curl --cacert {} https://{}:{}/'.format(cacert, host, port)
    # curl = 'wget --ca-certificate {} https://{}:{}/'.format(cacert, host, port)
    print(curl)
    print(os.system(curl))
    ctx.shutdown()


@fixture
def httpd_fixture(context):
    if not hasattr(context.scenario, "httpd"):
        context.scenario.httpd = HttpServerContext()

    yield context.scenario.httpd
    context.scenario.httpd.shutdown()
