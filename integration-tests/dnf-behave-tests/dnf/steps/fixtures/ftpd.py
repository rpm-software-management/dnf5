# -*- coding: utf-8 -*-

from behave import fixture
import os

from pyftpdlib.authorizers import DummyAuthorizer
from pyftpdlib.handlers import FTPHandler
from pyftpdlib.servers import FTPServer

from steps.fixtures.server import ServerContext


class NoLogFtpHandler(FTPHandler):
    def log_request(self, *args, **kwargs):
        pass


class FtpServerContext(ServerContext):
    """
    This object manages group of simple ftp servers. Each of them is run in
    separate process and serves configured directory.

    Usage:
    ctx = FtpServerContext()
    ctx.new_ftp_server('/path/to/directory/supposed/to/be/served')
    ctx.new_ftp_server('/path/to/other_directory/supposed/to/be/served')
    do_stuff()
    ctx.shutdown()
    """

    @staticmethod
    def ftp_server(address, path, error):
        try:
            os.chdir(path)
            authorizer = DummyAuthorizer()
            # Read only anonymous user
            authorizer.add_anonymous(path)

            handler = NoLogFtpHandler
            handler.authorizer = authorizer

            # with 'localhost' in the address, the FTPServer defaults to IPv6; the
            # ready check would then need to be opening an AF_INET6 socket...
            ftpd = FTPServer(('127.0.0.1', address[1]), handler)
            ftpd.serve_forever()
        except Exception as e:
            error.value = str(e)

    def new_ftp_server(self, path):
        return self._start_server(path, self.ftp_server)


@fixture
def ftpd_fixture(context):
    if not hasattr(context.scenario, "ftpd"):
        context.scenario.ftpd = FtpServerContext()

    yield context.scenario.ftpd
    context.scenario.ftpd.shutdown()
