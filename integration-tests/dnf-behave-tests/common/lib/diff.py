# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import print_function

from behave.formatter.ansi_escapes import escapes
import sys
PY3 = sys.version_info.major >= 3
if PY3:
    from itertools import zip_longest
else:
    from itertools import izip_longest as zip_longest


def print_lines_diff(expected, found, num_lines_equal=0):
    """ Prints a colored diff of two lists of strings.

    Parameters:
        expected: list of strings expected to find
        found: list of strings found
        num_lines_equal: a number that says how many strings at the beginning
            treat as equal even if they differ; a hack for correctly diffing
            outputs with repository syncing
    """
    # make this function work for lists and tuples in addition to strings
    expected = [" ".join(l) if type(l) in (list, tuple) else l for l in expected]
    found = [" ".join(l) if type(l) in (list, tuple) else l for l in found]

    left_width = len("expected")

    # calculate the width of the left column
    for line in expected:
        left_width = max(len(line), left_width)

    print("{:{left_width}}  |  {}".format("expected", "found", left_width=left_width))

    for line in zip_longest(expected, found, fillvalue=""):
        if num_lines_equal > 0 or line[0] == line[1]:
            color = escapes['passed_arg']
        else:
            color = escapes['failed_arg']
        print("{}{:{left_width}}  |  {}{}".format(
            color, line[0], line[1], escapes['reset'], left_width=left_width))
        num_lines_equal -= 1
