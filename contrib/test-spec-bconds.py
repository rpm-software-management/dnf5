#!/usr/bin/python3


import os
import subprocess
import sys


FLAGS = {
    "comps": {},
    "dnf5": {
        "with": ["libdnf-cli"],
    },
    "dnf5_plugins": {},
    "dnf5daemon_client": {
        "with": ["libdnf-cli"],
    },
    "dnf5daemon_server": {
        "with": ["libdnf-cli"],
    },
    "dnf5daemon_tests": {},
    "html": {},
    # "libdnf_cli": {},
    "modulemd": {},
    "perl5": {},
    "plugin_actions": {},
    "python3": {},
    "python_plugins_loader": {
        # requires python3
        "with": ["python3"],
    },
    "ruby": {},
    "static_libsolv": {},
    "zchunk": {},
}


FLAGS_ALWAYS_WITH = [
    "libdnf_cli",
    "tests",
]


FLAGS_ALWAYS_WITHOUT = [
    # we don't care about a different compiler now
    "clang",

    # broken option
    "go",

    # man pages require nearly all other options on
    "man",

    # we don't care about performance tests now
    "performance_tests",

    # we don't care about code sanity now
    "sanitizers",
]


TOPDIR = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
SPEC = os.path.abspath(os.path.join(TOPDIR, "dnf5.spec"))


def get_name_version():
    cmd = ["rpmspec", "-q", SPEC, "--srpm", "--qf", "%{name}-%{version}"]
    return subprocess.check_output(cmd, encoding="utf-8").strip()


def git_archive():
    """
    Create a tarball named exactly as the tarball from the spec,
    so we don't have to modify the spec at all.
    """
    nv = get_name_version()
    archive_path = f"{TOPDIR}/{nv}.tar.gz"
    cmd = [
        "git", "archive",
        "--prefix", f"{nv}/",
        "--output", archive_path,
        "HEAD"
    ]
    subprocess.run(cmd, check=True, cwd=TOPDIR)
    print(f"Created archive {archive_path}")


def install_build_deps():
    cmd = ["dnf", "-y", "builddep", SPEC]

    flags = []
    flags += list(FLAGS)
    flags += list(FLAGS_ALWAYS_WITH)
    for data in FLAGS.values():
        flags += data.get("with", [])

    for flag in flags:
        cmd += ["--define", f"_with_{flag} 1"]

    subprocess.run(cmd, check=True)


def build(with_flag):
    data = FLAGS[with_flag]

    cmd = ["rpmbuild", "--ba", SPEC]
    cmd += [
        # use tarball from git_archive()'s target location
        "--define", f"_sourcedir {TOPDIR}",
        # speedup: disable compression
        "--define", "_source_payload w.ufdio",
        "--define", "_binary_payload w.ufdio",
        # speedup: disable debuginfo
        "--define", "debug_package %{nil}",
    ]

    # enable the current flag
    cmd += ["--with", with_flag]

    # disable the other flags
    for without_flag in FLAGS:
        if without_flag == with_flag:
            continue

        # enable deps instead of disabling
        deps_with = data.get("with", [])
        if without_flag in deps_with:
            cmd += ["--with", without_flag]
            continue

        cmd += ["--without", without_flag]

    # enable the flags that should be always on
    for always_with_flag in FLAGS_ALWAYS_WITH:
        cmd += ["--with", always_with_flag]

    # disable the flags that should be always off
    for always_without_flag in FLAGS_ALWAYS_WITHOUT:
        cmd += ["--without", always_without_flag]

    print(10 * "\n")
    print(f"Building with flag '{with_flag}'...")
    print("Running command:", cmd)
    try:
        subprocess.run(cmd, check=True)
    except subprocess.CalledProcessError:
        print(f"Error: Failed to build with flag '{with_flag}'")
        return False
    return True


def main():
    git_archive()
    install_build_deps()

    errors = []
    for flag in FLAGS:
        success = build(flag)
        if not success:
            errors.append(flag)

    if errors:
        print(10 * "\n")
        print("Error: The following flags failed to build:", errors)
        sys.exit(1)


if __name__ == "__main__":
    main()
