import os
import sys
import subprocess


if os.environ.get("READTHEDOCS") != "True":
    print("This setup.py is supposed to be used only to build documentation in Read the Docs.")
    print("Use cmake && make doc instead to build documentation locally")
    sys.exit(1)


DIR = os.path.abspath(os.path.dirname(__file__))


def configure(src_path, dst_path, substitutions):
    print("Configuring {}...".format(src_path))

    # paths are relative to setup.py location
    src_path = os.path.join(DIR, src_path)
    dst_path = os.path.join(DIR, dst_path)

    data = open(src_path, "r").read()
    for pattern, substitution in substitutions.items():
        data = data.replace(pattern, substitution)
    open(dst_path, "w").write(data)


# configure the .in files
configure("Doxyfile.in", "Doxyfile", {"@CMAKE_SOURCE_DIR@": os.path.join(DIR, "..")})
configure("conf.py.in", "conf.py", {"@CMAKE_CURRENT_BINARY_DIR@": DIR})


# run doxygen manually
print("Running doxygen...")
subprocess.run(["doxygen"], cwd=DIR, check=True)


# no setup() is called
# this file only configures files for building docs in Read the Docs
