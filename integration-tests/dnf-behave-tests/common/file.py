# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import print_function

import behave
import glob
import re
import os

from common.lib.behave_ext import check_context_table
from common.lib.checksum import sha256_checksum
from common.lib.cmd import run
from common.lib.diff import print_lines_diff
from common.lib.file import copy_file
from common.lib.file import copy_tree
from common.lib.file import create_compressed_file_with_contents
from common.lib.file import create_file_with_contents
from common.lib.file import delete_directory
from common.lib.file import delete_file
from common.lib.file import ensure_directory_exists
from common.lib.file import ensure_file_exists
from common.lib.file import file_timestamp
from common.lib.file import prepend_installroot
from common.lib.file import read_file_contents
from common.lib.file import find_file_by_glob
from common.lib.text import lines_match_to_regexps_line_by_line


@behave.given('I create directory "{dirpath}"')
def step_impl(context, dirpath):
    full_path = prepend_installroot(context, dirpath)
    ensure_directory_exists(full_path)


@behave.given('I create file "{filepath}" with')
def step_impl(context, filepath):
    full_path = prepend_installroot(context, filepath)
    ensure_directory_exists(os.path.dirname(full_path))
    create_file_with_contents(full_path, context.text)


@behave.given('I create and substitute file "{filepath}" with')
def step_impl(context, filepath):
    full_path = prepend_installroot(context, filepath)
    ensure_directory_exists(os.path.dirname(full_path))
    create_file_with_contents(full_path, context.text.format(context=context))


@behave.given('I create symlink "{dst}" to file "{src}"')
def step_impl(context, dst, src):
    dst = prepend_installroot(context, dst)
    src = prepend_installroot(context, src)
    ensure_directory_exists(os.path.dirname(dst))
    os.symlink(src, dst)


@behave.given('I delete file "{filepath}"')
def step_delete_file(context, filepath):
    full_path = prepend_installroot(context, filepath)
    delete_file(full_path)


@behave.given('I delete file "{filepath}" with globs')
def step_delete_file_with_globs(context, filepath):
    for path in glob.glob(prepend_installroot(context, filepath)):
        delete_file(path)


@behave.given('I delete directory "{dirpath}"')
def step_delete_directory(context, dirpath):
    full_path = prepend_installroot(context, dirpath)
    delete_directory(full_path)


@behave.step('directory "{dirpath}" is empty')
def directory_is_empty(context, dirpath):
    full_path = prepend_installroot(context, dirpath)
    found = os.listdir(full_path)
    if len(found) > 0:
        raise AssertionError('Directory "{}" contains: \n{}'.format(full_path, '\n'.join(found)))


@behave.step('file "{filepath}" exists')
def file_exists(context, filepath):
    full_path = prepend_installroot(context, filepath)
    find_file_by_glob(full_path)


@behave.step('file "{filepath}" does not exist')
def file_does_not_exist(context, filepath):
    full_path = prepend_installroot(context, filepath)
    result = glob.glob(full_path)
    if len(result) > 0:
        raise AssertionError("Filepath %s matches existing files: \n%s" % (full_path, '\n'.join(result)))


@behave.step('file "{filepath}" contents is')
def file_contents_is(context, filepath):
    expected = context.text.strip()
    full_path = prepend_installroot(context, filepath)
    f = find_file_by_glob(full_path)
    found = read_file_contents(f).strip()
    if expected == found:
        return
    print_lines_diff(expected.split('\n'), found.split('\n'))
    raise AssertionError("File '{}' contents is different then expected.".format(filepath))


@behave.then('file "{filepath}" matches line by line')
def then_stdout_matches_line_by_line(context, filepath):
    """
    Checks that each line of given file matches respective line in regular expressions.
    """
    expected = context.text.split('\n')
    full_path = prepend_installroot(context, filepath)
    f = find_file_by_glob(full_path)
    found = read_file_contents(f).split('\n')

    lines_match_to_regexps_line_by_line(found, expected)


@behave.step('file "{filepath}" contains lines')
def file_contains(context, filepath):
    regexp_lines = context.text.split('\n')
    full_path = prepend_installroot(context, filepath)
    ensure_directory_exists(os.path.dirname(full_path))
    read_str = read_file_contents(full_path)
    for line in regexp_lines:
        if not re.search(line, read_str):
            print("line: " + line + " not found")
            raise AssertionError("File %s contains: \n%s" % (filepath, read_str))
    return


@behave.step('file "{filepath}" does not contain lines')
def file_does_not_contain(context, filepath):
    regexp_lines = context.text.split('\n')
    full_path = prepend_installroot(context, filepath)
    ensure_directory_exists(os.path.dirname(full_path))
    read_str = read_file_contents(full_path)
    for line in regexp_lines:
        if re.search(line, read_str):
            print("line: " + line + " found")
            raise AssertionError("File %s contains: \n%s" % (filepath, read_str))
    return


@behave.step('I copy directory "{source}" to "{destination}"')
def step_impl(context, source, destination):
    source = source.format(context=context)
    destination = prepend_installroot(context, destination)
    ensure_directory_exists(os.path.dirname(destination))
    copy_tree(source, destination)


@behave.step('I copy file "{source}" to "{destination}"')
def copy_file_to(context, source, destination):
    source = source.format(context=context)
    destination = destination.format(context=context)
    destination = prepend_installroot(context, destination)
    ensure_directory_exists(os.path.dirname(destination))
    # If we dont specify destination with name keep the name of source file
    if (os.path.isdir(destination)):
        destination = os.path.join(destination, os.path.basename(source))
    copy_file(source, destination)


@behave.step('the files "{first}" and "{second}" do not differ')
def step_impl(context, first, second):
    first = first.format(context=context)
    second = second.format(context=context)
    ensure_file_exists(first)
    ensure_file_exists(second)
    cmd = "diff -r {} {}".format(first, second)
    exitcode, _, _ = run(cmd, shell=True)
    assert exitcode == 0, 'Files "{}" and "{}" differ.'.format(first, second)


@behave.step('the text file contents of "{first}" and "{second}" do not differ')
def step_impl(context, first, second):
    full_path_first = prepend_installroot(context, first)
    full_path_second = prepend_installroot(context, second)
    f1 = find_file_by_glob(full_path_first)
    f2 = find_file_by_glob(full_path_second)
    found1 = read_file_contents(f1).strip()
    found2 = read_file_contents(f2).strip()
    if found1 == found2:
        return
    print_lines_diff(found1.split('\n'), found2.split('\n'))
    raise AssertionError("File '{}' contents differ from {}.".format(first, second))


@behave.step('timestamps of the files "{first}" and "{second}" do not differ')
def step_impl(context, first, second):
    first = first.format(context=context)
    second = second.format(context=context)
    ensure_file_exists(first)
    ensure_file_exists(second)
    # strip the fractional part of timestamps as the precision of timestamps
    # in http headers is only in seconds.
    ts_first = int(file_timestamp(first))
    ts_second = int(file_timestamp(second))
    assert ts_first == ts_second, \
        'Timestamps of files "{}": {} and "{}": {} are differt.'.format(
            first, ts_first, second, ts_second)


@behave.step('size of file "{filepath}" is at most "{expected_size}"')
def file_size_less_than(context, filepath, expected_size):
    full_path = prepend_installroot(context, filepath)
    size = os.path.getsize(full_path)
    assert size <= int(expected_size), 'File "{}" has size "{}"'.format(full_path, size)


@behave.then("file sha256 checksums are following")
def then_file_sha256_checksums_are_following(context):
    check_context_table(context, ["Path", "sha256"])

    for path, checksum in context.table:
        path = path.format(context=context)

        # "-" checksum indicates that file must not exist
        if checksum == "-":
            if os.path.isfile(path):
                raise AssertionError("An unexpected file found on disk: %s" % path)
            else:
                continue

        file_checksum = sha256_checksum(open(path, "rb").read())

        # when 'file://<path>' is provided instead of the checksum,
        # the sha256 value is computed from that file
        if checksum.startswith("file://"):
            checksum_path = checksum[7:]
            checksum_path = checksum_path.format(context=context)
            checksum = sha256_checksum(open(checksum_path, "rb").read())

        if file_checksum != checksum:
            raise AssertionError("File sha256 checksum doesn't match (expected: %s, actual: %s): %s" %
                                 (checksum, file_checksum, path))


@behave.step("I create \"{compression}\" compressed file \"{filepath}\" with")
def create_compressed_file_with(context, compression, filepath):
    target_path = prepend_installroot(context, filepath)
    create_compressed_file_with_contents(target_path, compression, context.text)


@behave.step("I compress file \"{filepath}\" using \"{compression}\"")
def create_compressed_file_with(context, compression, filepath):
    file_path = prepend_installroot(context, filepath)
    ensure_file_exists(file_path)
    create_compressed_file_with_contents(
        file_path, compression, read_file_contents(file_path))
