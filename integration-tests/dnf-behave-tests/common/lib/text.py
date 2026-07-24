# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import print_function

import re


def lines_match_to_regexps_line_by_line(out_lines, regexp_lines):
    """Matches list of lines against a list of regexps line by line"""
    while out_lines:
        line = out_lines.pop(0)
        if line and (not regexp_lines):  # there is no remaining regexp
            raise AssertionError("Not having a regexp to match line '%s'" % line)
        elif regexp_lines:
            regexp = regexp_lines.pop(0).strip()
            while regexp.startswith('?'):
                if not re.search(regexp[1:], line):  # optional regexp that doesn't need to be matched
                    if regexp_lines:
                        regexp = regexp_lines.pop(0).strip()
                    else:
                        raise AssertionError("Not having a regexp to match line '%s'" % line)
                else:
                    regexp = regexp[1:]
            if regexp:
                if not re.search(regexp, line):
                    raise AssertionError("'%s' regexp does not match line: '%s'" % (regexp, line))

            else:
                if not line == "":
                    raise AssertionError("'%s' is not empty line" % line)

    if regexp_lines:  # there are some unprocessed regexps
        raise AssertionError("No more line to match regexp '%s'" % regexp_lines[0])
