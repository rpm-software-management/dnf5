# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import print_function

import codecs
import os
import shutil
import glob
import subprocess

import bz2
import gzip
# xz compression
try:
    import lzma
except ImportError:
    from backports import lzma


def prepend_installroot(context, path):
    path = path.format(context=context)
    root = '/'
    # tests tend to have a directory for temporary files, make it the default
    # root directory for the file manipulation steps unless the path starts
    # with '//'
    if hasattr(context.scenario, "default_tmp_dir") and not path.startswith('//'):
        root = context.scenario.default_tmp_dir
    return os.path.join(root, path.lstrip("/"))


def find_file_by_glob(filepath):
    result = glob.glob(filepath)
    if len(result) > 1:
        raise AssertionError("File path %s matches multiple files: \n%s" % (filepath, '\n'.join(result)))
    elif len(result) < 1:
        raise AssertionError("File path %s doesn't match any file." % (filepath))

    return result[0]


def ensure_directory_exists(dirname):
    if not os.path.exists(dirname):
        os.makedirs(dirname)
    assert os.path.exists(dirname), "ENSURE: dir exists {!r}".format(dirname)
    assert os.path.isdir(dirname), "ENSURE: is a dir {!r}".format(dirname)


def ensure_file_exists(filename):
    assert os.path.exists(filename), "ENSURE: file exists {!r}".format(filename)


def delete_file(filename):
    if os.path.exists(filename):
        os.remove(filename)


def delete_directory(dirname):
    if os.path.exists(dirname):
        shutil.rmtree(dirname)


def create_file_with_contents(filename, contents, encoding="utf-8"):
    if os.path.exists(filename):
        os.remove(filename)
    with codecs.open(filename, "w", encoding) as outstream:
        outstream.write(contents)
        outstream.flush()
    assert os.path.exists(filename), "ENSURE: file exists {!r}".format(filename)


def read_file_contents(filename, encoding="utf-8"):
    assert os.path.exists(filename), "ENSURE: file exists {!r}".format(filename)

    output = decompress_file_by_extension(filename)
    if output is not None:
        return output.decode(encoding)

    with codecs.open(filename, "r", encoding) as outstream:
        output = outstream.read()
        return output


def copy_tree(source, destination):
    shutil.copytree(source, destination)
    assert os.path.exists(destination), "copy_tree {} -> {} failed".format(source, destination)


def copy_file(source, destination):
    shutil.copyfile(source, destination)
    assert os.path.exists(destination), "copy_tree {} -> {} failed".format(source, destination)


def file_timestamp(filename):
    return os.path.getmtime(filename)


def decompress_file_by_extension(src):
    if src.endswith(".bz2"):
        return bz2.open(src, "rb").read()
    elif src.endswith(".gz"):
        return gzip.open(src, "rb").read()
    elif src.endswith(".xz"):
        return lzma.open(src, "rb").read()
    elif src.endswith(".zst"):
        return subprocess.run(["unzstd", "--stdout", src], capture_output=True).stdout
    elif src.endswith(".zck"):
        return subprocess.run(["unzck", "--stdout", src], capture_output=True).stdout

    return None


def get_compression_suffix(type_str):
    if type_str in ("gz", "zck", "xz", "bz2"):
        return "." + type_str
    if type_str == "zstd":
        return ".zst"
    if type_str == "-":
        return ""
    raise ValueError("Unknown compression type: " + type_str)


def create_compressed_file_with_contents(filename, compression, contents, encoding="utf-8"):
    fullname = filename + get_compression_suffix(compression)
    if os.path.exists(fullname):
        raise ValueError("File: " + fullname + " already exists")

    if compression == "gz":
        with gzip.open(fullname, 'wt') as f:
            f.write(contents)
    elif compression == "xz":
        with lzma.open(fullname, 'wt') as f:
            f.write(contents)
    elif compression == "bz2":
        with bz2.open(fullname, 'wt') as f:
            f.write(contents)
    else:
        raise ValueError("Unknown compression type: " + compression)
