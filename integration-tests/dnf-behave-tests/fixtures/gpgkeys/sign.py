import os
import shutil
import subprocess
import tempfile
from dataclasses import dataclass
from pathlib import Path
from datetime import datetime, timedelta


@dataclass
class KeySpec:
    name: str
    packages: list[str]
    # Config
    sign_subkey: bool = False
    noeof_keys: bool = False
    expiration_date: bool = False
    unsigned: bool = False


def run_cmd(cmd: list[str], env: dict[str, str] | None = None) -> subprocess.CompletedProcess[str]:
    """Helper to run shell commands with error handling."""
    try:
        print(f"+ {' '.join(cmd)}")
        res = subprocess.run(cmd, env=env, check=True, capture_output=True, text=True)
        return res
    except subprocess.CalledProcessError as e:
        print()
        print(f"stdout:\n{e.stdout}")
        print(f"stderr:\n{e.stderr}")
        raise e


def process_key_spec(key_spec: KeySpec) -> None:
    # Skip unsigned packages
    if key_spec.unsigned:
        return

    key_name = key_spec.name

    # kill gpg-agent because path to gpg keys changes in each iteration
    run_cmd(["gpgconf", "--kill", "gpg-agent"])

    key_dir = keys_root / key_name
    key_dir.mkdir()

    # workaround for gpgme unable to handle long paths
    # * create a temp directory /tmp/<tempdir>
    # * symlink key directory -> /tmp/<tempdir>/gpghome
    #
    # gpg usually works as stated in https://bugzilla.redhat.com/show_bug.cgi?id=1813705#c3
    # but sometimes (in containers without running systemd) /run/user/$UID doesn't exist
    # and that's why this workaround is needed
    with tempfile.TemporaryDirectory() as tmp_dir:
        tmp_dir_path = Path(tmp_dir)
        tmp_key_dir = tmp_dir_path / "gpghome"
        os.symlink(key_dir, tmp_key_dir)
        # keys are without expiration date by default
        # if expiration is requested, set it to 1 year from now
        expiry_date = "0"
        if key_spec.expiration_date:
            expiry_date = (datetime.now() + timedelta(days=365)).strftime("%Y-%m-%d")

        # Setup environment for GPG
        env = os.environ.copy()
        env["HOME"] = str(tmp_key_dir)

        # create key (without password)
        run_cmd([
            "gpg2", "--batch", "--passphrase", "",
            "--quick-gen-key", key_name, "default", "default", expiry_date
        ], env=env)

        # Add signing subkey if requested
        if key_spec.sign_subkey:
            res = run_cmd(["gpg2", "--list-keys", "--with-colons", key_name], env=env)
            # Find the first fingerprint (fpr) line
            fpr = next(line.split(":")[9] for line in res.stdout.splitlines() if line.startswith("fpr:"))
            run_cmd([
                "gpg2", "--batch", "--passphrase", "",
                "--quick-add-key", fpr, "default", "sign", "0"
            ], env=env)

        # Export keys
        pub_key_path = tmp_key_dir / f"{key_name}-public"
        priv_key_path = tmp_key_dir / f"{key_name}-private"

        with open(pub_key_path, "w") as f:
            f.write(run_cmd(["gpg2", "--export", "-a", key_name], env=env).stdout)

        with open(priv_key_path, "w") as f:
            f.write(run_cmd(["gpg2", "--export-secret-keys", "-a", key_name], env=env).stdout)

        # Remove trailing EOF if requested
        if key_spec.noeof_keys:
            for p in [pub_key_path, priv_key_path]:
                with open(p, "rb+") as f:
                    f.seek(-1, os.SEEK_END)
                    f.truncate()

        # Create .rpmmacros
        rpmmacros_content = (
            f"%_signature gpg\n"
            f"%_gpg_name {key_name}"
        )
        (tmp_key_dir / ".rpmmacros").write_text(rpmmacros_content)

        # Sign packages
        for pkg_path in key_spec.packages:
            run_cmd(["rpm", "--addsign", str(pkg_path)], env=env)


# Every other package not specified here is signed by "default-key"
key_specs = [
    KeySpec(
        name="dnf-ci-gpg",
        packages=[
            "dnf-ci-gpg/noarch/setup-2.12.1-1.fc29.noarch.rpm",
            "dnf-ci-gpg/noarch/abcde-2.9.2-1.fc29.noarch.rpm",
            "dnf-ci-gpg/noarch/broken-package-0.2.4-1.fc29.noarch.rpm",
            "dnf-ci-gpg/x86_64/glibc-2.28-9.fc29.x86_64.rpm",
            "dnf-ci-gpg/x86_64/glibc-common-2.28-9.fc29.x86_64.rpm",
            "dnf-ci-gpg/x86_64/glibc-all-langpacks-2.28-9.fc29.x86_64.rpm",
            "dnf-ci-gpg-updates/noarch/basesystem-11-6.fc29.noarch.rpm",
        ]
    ),
    KeySpec(
        name="dnf-ci-gpg-updates",
        packages=[
            "dnf-ci-gpg/noarch/basesystem-11-6.fc29.noarch.rpm",
            "dnf-ci-gpg-updates/x86_64/wget-2.0.0-1.fc29.x86_64.rpm"
        ]
    ),
    KeySpec(
        name="dnf-ci-gpg-subkey",
        sign_subkey=True,
        packages=[
            "dnf-ci-gpg/x86_64/filesystem-3.9-2.fc29.x86_64.rpm",
            "dnf-ci-gpg/x86_64/filesystem-content-3.9-2.fc29.x86_64.rpm"
        ]
    ),
    KeySpec(
        name="dnf-ci-gpg-noeol",
        noeof_keys=True,
        packages=[
            "dnf-ci-gpg-noeol/noarch/abcde-2.9.2-1.fc29.noarch.rpm",
            "dnf-ci-gpg-noeol/x86_64/wget-1.19.5-5.fc29.x86_64.rpm"
        ]
    ),
    KeySpec(
        name="dnf-ci-gpg-expiry",
        expiration_date=True,
        packages=[
            "dnf-ci-gpg-expiry/x86_64/wget-1.19.5-5.fc29.x86_64.rpm"
        ]
    ),
    KeySpec(
        name="reposync-gpg",
        packages=[
            "reposync-gpg/x86_64/dedalo-signed-1.0-1.fc29.x86_64.rpm",
            "reposync-gpg/src/dedalo-signed-1.0-1.fc29.src.rpm"
        ]
    ),
    KeySpec(
        name="",
        unsigned=True,
        packages=[
            "reposync-gpg/x86_64/dedalo-unsigned-1.0-1.fc29.x86_64.rpm",
            "reposync-gpg/src/dedalo-unsigned-1.0-1.fc29.src.rpm",
            "dnf-ci-gpg/x86_64/flac-1.3.2-8.fc29.x86_64.rpm",
            "unsigned/x86_64/sarcina-1.0-1.fc29.x86_64.rpm",
            "unsigned/x86_64/sarcina-2.0-1.fc29.x86_64.rpm",
            "unsigned/x86_64/kernel-1.0-1.fc29.x86_64.rpm",
            "unsigned/x86_64/kernel-2.0-1.fc29.x86_64.rpm",
            "unsigned/x86_64/glibc-1.0-1.fc29.x86_64.rpm",
            "unsigned/x86_64/glibc-2.0-1.fc29.x86_64.rpm"
        ]
    ),
]

# Setup directories
script_dir = Path(__file__).parent.resolve()
repo_dir = Path(script_dir.parent / "repos")
keys_root = script_dir / "keys"

# Cleanup and recreate keys directory
if keys_root.exists():
    shutil.rmtree(keys_root)
keys_root.mkdir()

all_rpm_files = set(repo_dir.rglob('*.rpm'))

for key_spec in key_specs:
    key_spec.packages = set(repo_dir / Path(item) for item in key_spec.packages)
    all_rpm_files = all_rpm_files - key_spec.packages
    process_key_spec(key_spec)

default_key = KeySpec(
    name="default-key",
    packages=all_rpm_files
)

process_key_spec(default_key)
