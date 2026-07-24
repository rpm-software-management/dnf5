# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import print_function

import behave
import os
import subprocess

# make sure the server_type is registered in behave
import steps.server

from common.lib.behave_ext import check_context_table
from common.lib.checksum import sha256_checksum
from common.lib.cmd import run_in_context
from common.lib.diff import print_lines_diff
from common.lib.file import (
    copy_tree,
    create_file_with_contents,
    delete_file,
    delete_directory,
    ensure_directory_exists,
    ensure_file_exists,
    find_file_by_glob,
)
from common.lib.os_version import want_modularity
from fixtures import start_server_based_on_type, stop_server_type
from fixtures.osrelease import osrelease_fixture
from fixtures.machineid import machineid_fixture


def repo_config(repo, new={}):
    config = {
        "name": repo + " test repository",
        "enabled": "1",
    }
    config.update(new)
    return config


def write_repo_config(context, repo, config, path=None):
    path = path or os.path.join(context.dnf.installroot, "etc/yum.repos.d/")

    conf_text = "[%s]\n" % repo
    for key, value in config.items():
        if value:
            conf_text += ("%s=%s\n" % (key, value)).format(repo=repo, context=context)

    create_file_with_contents(os.path.join(path, repo + ".repo"), conf_text)


class RepoInfo(object):
    def __init__(self, context, repo):
        self.active = False
        self.path = os.path.join(context.scenario.repos_location, repo)
        self.config = repo_config(repo, {"baseurl": "file://" + self.path})
        self.copied = False

    def update_config(self, new_conf):
        self.config.update(new_conf)

    def get_substituted_path(self, context):
        return self.path.replace("$releasever", context.dnf.releasever)


def get_repo_info(context, repo):
    return context.dnf.repos.setdefault(repo, RepoInfo(context, repo))


def create_repo_conf(context, repo):
    repo_info = get_repo_info(context, repo)
    repo_info.active = True

    write_repo_config(context, repo, repo_info.config)


def generate_repodata(context, repo, extra_args=None, explicit=False, can_fail=False, expected_exit_code=None):
    repo_subst = repo.replace("$releasever", context.dnf.releasever)
    repo_info = get_repo_info(context, repo)

    if repo_subst in context.repos and not extra_args and not (explicit and repo_info.copied):
        return

    args = "--no-database --simple-md-filenames --revision=1550000000"

    groups_filename = os.path.join(context.dnf.fixturesdir, "specs", repo_subst, "comps.xml")
    if os.path.isfile(groups_filename):
        args += " --groupfile " + groups_filename

    if extra_args is not None:
        args += " " + extra_args

    target_path = repo_info.get_substituted_path(context)
    if not os.path.isdir(target_path):
        os.makedirs(target_path)

    run_in_context(context, "createrepo_c %s '%s'" % (args, target_path), can_fail, expected_exit_code)

    repodata_path = os.path.join(target_path, "repodata")

    updateinfo_filename = os.path.join(context.dnf.fixturesdir, "specs", repo_subst, "updateinfo.xml")
    if os.path.isfile(updateinfo_filename):
        run_in_context(context, "modifyrepo_c %s '%s'" % (updateinfo_filename, repodata_path))

    if want_modularity():
        modules_filename = os.path.join(context.dnf.fixturesdir, "specs", repo_subst, "modules.yaml")
        if os.path.isfile(modules_filename):
            run_in_context(context, "modifyrepo_c --mdtype=modules %s '%s'" % (modules_filename, repodata_path))

    if not repo_info.copied:
        context.repos[repo_subst] = True


def generate_metalink(destdir, url):
    metalink = """<?xml version="1.0" encoding="utf-8"?>
<metalink version="3.0" xmlns="http://www.metalinker.org/" xmlns:mm0="http://fedorahosted.org/mirrormanager">
  <files>
    <file name="repomd.xml">
      <mm0:timestamp>1550000000</mm0:timestamp>
      <size>{size}</size>
      <verification>
        <hash type="sha256">{csum}</hash>
      </verification>
      <resources>
        <url protocol="{schema}" type="{schema}">{url}/repodata/repomd.xml</url>
      </resources>
    </file>
  </files>
</metalink>
"""
    schema = url.split(':')[0]
    with open(os.path.join(destdir, 'repodata', 'repomd.xml')) as f:
        repomd = f.read()
    with open(os.path.join(destdir, 'metalink.xml'), 'w') as f:
        data = metalink.format(
            size=len(repomd),
            csum=sha256_checksum(repomd.encode('utf-8')),
            schema=schema,
            url=url,
        )
        f.write(data + '\n')


@behave.step("I set releasever to \"{releasever}\"")
def step_impl(context, releasever):
    context.dnf._set("releasever", releasever)
    for repo, info in context.dnf.repos.items():
        if "$releasever" in repo:
            generate_repodata(context, repo)


@behave.step("I generate repodata for repository \"{repo}\" with extra arguments \"{args}\"")
def step_generate_repodata_for_repository_with_extra_args(context, repo, args):
    """
    Generates the repository repodata (without configuring it for use),
    allowing to specify extra arguments to createrepo_c. Only possible if the
    repository was copied for modification.
    """
    repo_info = get_repo_info(context, repo)
    assert repo_info.copied, \
        "Cannot specify extra createrepo_c arguments if the repository wasn't copied for modification"

    generate_repodata(context, repo, extra_args=args.format(context=context), explicit=True)


@behave.step("I generate repodata for repository \"{repo}\"")
def step_generate_repodata_for_repository(context, repo):
    """
    Generates the repository repodata (without configuring it for use).
    """
    generate_repodata(context, repo, explicit=True)


@behave.step("I use repository \"{repo}\" generated with exit code \"{exitcode}\"")
def step_use_repository_generated_with_exit_code(context, repo, exitcode):
    """
    Generates the repodata if they weren't generated yet for this run of
    behave. Creates the repository's config file at /etc/yum.repos.d/ (inside
    installroot).
    Createrepo_c can fail during generating the repodata, with given exit code.
    """
    generate_repodata(context, repo, can_fail=True, expected_exit_code=int(exitcode))
    create_repo_conf(context, repo)


@behave.step("I use repository \"{repo}\"")
def step_use_repository(context, repo):
    """
    Generates the repodata if they weren't generated yet for this run of
    behave. Creates the repository's config file at /etc/yum.repos.d/ (inside
    installroot).
    """
    generate_repodata(context, repo)
    create_repo_conf(context, repo)


@behave.step("I configure repository \"{repo}\" with")
def step_configure_repository(context, repo):
    """
    Sets the repository configuration (i.e. the contents of its config file).
    If the repository is used, overwrites its config file with the new
    configuration.
    """
    check_context_table(context, ["key", "value"])

    repo_info = get_repo_info(context, repo)
    repo_info.update_config(dict(context.table))
    if repo_info.active:
        create_repo_conf(context, repo)


@behave.step("I use repository \"{repo}\" with configuration")
def step_use_repository_with_config(context, repo):
    """
    Generates the repodata if they weren't generated yet for this run of
    behave. Sets the repository configuration (i.e. the contents of its config
    file) and creates its config file at /etc/yum.repos.d/ (inside
    installroot).
    """
    check_context_table(context, ["key", "value"])

    generate_repodata(context, repo)
    get_repo_info(context, repo).update_config(dict(context.table))
    create_repo_conf(context, repo)


@behave.step("I drop repository \"{repo}\"")
def step_drop_repository(context, repo):
    """
    Deletes the repository's config file from /etc/yum.repos.d/ (inside installroot).
    """
    assert repo in context.dnf.repos, 'Repository "%s" was never used.' % repo

    delete_file(os.path.join(context.dnf.installroot, "etc/yum.repos.d/", repo + ".repo"))
    get_repo_info(context, repo).active = False


@behave.step("I copy repository \"{repo}\" for modification")
def step_copy_repository(context, repo):
    """
    Copies the whole contents of the repository directory (i.e. the packages
    and repodata) to a temp directory of the current scenario. Use this if you
    need to modify the data of this directory, so that the original repository
    data stay unchanged for the other tests.
    """
    generate_repodata(context, repo)

    repo_info = get_repo_info(context, repo)

    src_path = repo_info.get_substituted_path(context)
    repo_info.path = os.path.join(context.dnf.tempdir, "repos", repo)
    copy_tree(src_path, repo_info.get_substituted_path(context))

    repo_info.copied = True
    repo_info.update_config({"baseurl": "file://" + repo_info.path})


@behave.step("I configure a new repository \"{repo}\" in \"{path}\" with")
def step_configure_new_repository_in(context, repo, path):
    """
    Creates a new repository config at `path` with the default values overriden
    with what is in the context table.
    """
    check_context_table(context, ["key", "value"])
    path = path.format(context=context)
    ensure_directory_exists(path)

    write_repo_config(context, repo, repo_config(repo, dict(context.table)), path)


@behave.step("I configure a new repository \"{repo}\" with")
def step_configure_new_repository(context, repo):
    """
    Creates a new repository config at the default location (/etc/yum.repos.d/
    inside installroot) with the default values overriden with what is in the
    context table.
    """
    check_context_table(context, ["key", "value"])

    write_repo_config(context, repo, repo_config(repo, dict(context.table)))


@behave.step("I make packages from repository \"{repo}\" accessible via {rtype:server_type}")
def make_repo_packages_accessible(context, repo, rtype):
    """
    Starts a new HTTP/FTP server at the repository's location and saves
    its port to context.
    """
    repo_info = get_repo_info(context, repo)
    host, port = start_server_based_on_type(context, repo_info.get_substituted_path(context), rtype)
    context.dnf.ports[repo] = port


@behave.step("I use repository \"{repo}\" as {rtype:server_type}")
def step_use_repository_as(context, repo, rtype):
    """
    Starts a new HTTP/FTP server at the repository's location and then
    configures the repository's baseurl with the server's url. Also generates
    the repodata if they weren't generated yet for this run of behave.
    """
    repo_info = get_repo_info(context, repo)
    server_dir = repo_info.get_substituted_path(context)

    if rtype == "https":
        certs = {
            "cacert": os.path.join(context.dnf.fixturesdir, 'certificates/testcerts/ca/cert.pem'),
            "cert": os.path.join(context.dnf.fixturesdir, 'certificates/testcerts/server/cert.pem'),
            "key": os.path.join(context.dnf.fixturesdir, 'certificates/testcerts/server/key.pem'),
        }
        host, port = start_server_based_on_type(context, server_dir, rtype, certs)
    else:
        host, port = start_server_based_on_type(context, server_dir, rtype)

    config = {
        "baseurl": "{}://{}:{}/".format(rtype, host, port)
    }

    if rtype == "https":
        client_ssl = context.dnf._get("client_ssl")

        config["sslcacert"] = certs["cacert"]
        if client_ssl:
            config["sslclientcert"] = client_ssl["certificate"]
            config["sslclientkey"] = client_ssl["key"]

    context.dnf.ports[repo] = port

    repo_info.update_config(config)
    generate_repodata(context, repo)
    create_repo_conf(context, repo)


@behave.step("I stop {rtype:server_type} server for repository \"{repo}\"")
def step_stop_server_for_repo(context, rtype, repo):
    """
    Stops the server that is running for the repository.
    """
    repo_info = get_repo_info(context, repo)
    stop_server_type(context, repo_info.get_substituted_path(context), rtype)


@behave.step("I set up metalink for repository \"{repo}\"")
def step_set_up_metalink_for_repository(context, repo):
    """
    Generates a metalink for a repository and configures the repository with
    the 'metalink' config option, which points to the newly created file.

    Note that you need to copy the repository using the "I copy repository for
    modification" step beforehand and if you're using a HTTP server, the
    sequence of steps needs to be this:
      I copy repository "foo" for modification
      I use repository "foo" as http
      I set up metalink for repository "foo"
    """
    repo_info = get_repo_info(context, repo)
    assert repo_info.path.startswith(context.dnf.tempdir), \
        "Creating a metalink needs to be done on a repo that was copied for modification."

    url = repo_info.config['baseurl']
    generate_metalink(repo_info.path, url)
    repo_info.update_config({
        "baseurl": "",
        "metalink": url + "metalink.xml?releasever=$releasever",
    })
    create_repo_conf(context, repo)


@behave.step("I am running a system identified as the \"{system}\"")
def given_system(context, system):
    behave.use_fixture(osrelease_fixture, context)
    system = system.split(';')
    distro = system[0]
    variant = None
    if len(system) > 1:
        variant = system[1]
        variant = '"{0}"'.format(variant.strip())
    name, version = distro.rsplit(' ', 1)
    name = '"{0}"'.format(name.strip())
    version = '"{0}"'.format(version.strip())
    data = dict(zip(('NAME', 'VERSION_ID', 'VARIANT_ID'),
                    (name, version, variant)))
    context.scenario.osrelease.set(data)


@behave.step("I remove the os-release file")
def given_no_osrelease(context):
    behave.use_fixture(osrelease_fixture, context)
    context.scenario.osrelease.delete()


@behave.step("the machine-id file is {what} as of {when}")
def step_machine_id_file(context, what, when):
    behave.use_fixture(machineid_fixture, context)
    machineid = context.scenario.machineid
    if when.startswith('on '):
        when = when[3:]
    if what == 'initialized':
        machineid.initialize(when)
    elif what == 'uninitialized':
        machineid.uninitialize(when)
    elif what == 'empty':
        machineid.empty()
    elif what == 'absent':
        machineid.delete()


@behave.step("I invalidate solvfile version of \"{path}\"")
def rewrite_solvfile_version(context, path):
    path = path.format(context=context)
    path = find_file_by_glob(path)
    ensure_file_exists(path)
    fp = open(path, "r+b")
    # Seek 40 bytes from start of file to the beginning userdata section
    fp.seek(40, 0)
    userdata_dnf_magic = fp.read(4)
    expected_dnf_magic = b'\x00dnf'
    assert userdata_dnf_magic == expected_dnf_magic, \
        'Provided solvfile doesn\'t contain expected userdata section with dnf magic bytes' \
        '(found "%s" vs. expected "%s")' % (userdata_dnf_magic, expected_dnf_magic)

    userdata_dnf_version = fp.read(4)
    expected_dnf_version = b'\x001.0'
    assert userdata_dnf_version == expected_dnf_version, \
        'Provided solvfile doesn\'t contain expected userdata dnf version, ' \
        '(found "%s" vs. expected "%s")' % (userdata_dnf_version, expected_dnf_version)

    fp.write(bytes("\0\0\0bogus", 'utf-8'))
    fp.close()


@behave.step("I sign repository \"{repo}\" metadata with \"{key_path}\"")
def step_sign_metadata_with_key(context, repo, key_path):
    """
    Sign repo with key (creates repomd.xml.asc).
    The signing is done in a temp directory so that the key is not imported
    after finishing.

    Note that you need to copy the repository using the "I copy repository for
    modification" step beforehand.
    """

    repo_info = get_repo_info(context, repo)
    assert repo_info.path.startswith(context.dnf.tempdir), \
        "Signing needs to be done on a repo that was copied for modification."

    temp_gpg_home = os.path.join(context.dnf.installroot, "gpg_home")
    ensure_directory_exists(temp_gpg_home)

    key_path = key_path.format(context=context)
    # import key into temp dir
    gpg_import_cmd = [
        "gpg",
        "--homedir",
        temp_gpg_home,
        "--import",
        key_path
    ]
    subprocess.run(gpg_import_cmd, check=True)

    repomd_path = os.path.join(repo_info.path, "repodata/repomd.xml")
    # sign repomd.xml
    gpg_cmd = [
        "gpg",
        "--homedir",
        temp_gpg_home,
        "--detach-sign",
        "--armor",
        repomd_path
    ]
    subprocess.run(gpg_cmd, check=True)

    delete_directory(temp_gpg_home)
