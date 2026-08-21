# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import print_function


def check_context_table(context, headings):
    if not context.table:
        raise ValueError("Table not specified.")

    if context.table.headings != headings:
        raise ValueError("Invalid table headings. Expected: %s" % ", ".join(headings))
