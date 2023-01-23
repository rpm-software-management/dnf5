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


def generate_bindings_from_dir(in_dir, out_dir):
    swig_options = ["-python", "-DSWIGWORDSIZE64", "-doxygen", "-relativeimport", "-outdir", out_dir, "-c++"]
    swig_includes = ["-I" + os.path.join(DIR, "../include"), "-I" + os.path.join(DIR, "../common"), "-I/usr/include/python3.11"]

    for in_file in os.listdir(in_dir):
        # exclude shared.i which is not a standalone interface file
        # it is included in other files.
        if in_file.endswith(".i") and in_file != "shared.i":
            print("Generating bindings for: " + in_file)
            subprocess.run(["/usr/bin/swig"] + swig_options + ["-interface", "_" + in_file[:-2]] + swig_includes + [os.path.join(in_dir, in_file)], cwd=DIR, check=True)


# configure the .in files
configure("Doxyfile.in", "Doxyfile", {"@CMAKE_SOURCE_DIR@": os.path.join(DIR, "..")})
configure("conf.py.in", "conf.py", {"@CMAKE_CURRENT_BINARY_DIR@": DIR})


# run doxygen manually
print("Running doxygen...")
subprocess.run(["doxygen"], cwd=DIR, check=True)


# run swig manually to generate Python bindings which are then used to generate Python API docs
print("Running SWIG...")

# libdnf5
# Generate bindings outside of doc dir, into their python bindings dir. This has to match with path provided to autoapi_dirs in conf.py.in
generate_bindings_from_dir(os.path.join(DIR + "/../bindings/libdnf5"), os.path.join(DIR + "/../bindings/python3/libdnf5"))

# libdnf5-cli
generate_bindings_from_dir(os.path.join(DIR + "/../bindings/libdnf5_cli"), os.path.join(DIR + "/../bindings/python3/libdnf5_cli"))

# no setup() is called
# this file only configures files for building docs in Read the Docs
