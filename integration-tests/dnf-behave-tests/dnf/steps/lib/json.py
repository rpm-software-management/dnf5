# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import print_function

from behave.formatter.ansi_escapes import escapes
import fnmatch


def diff_recursive(parent, obj1, obj2):
    problems = []
    if isinstance(obj1, list) and isinstance(obj2, list):
        # The list was originally a dictionary
        if (len(obj1) != 0 and isinstance(obj1[0], tuple)) or len(obj2) != 0 and isinstance(obj2[0], tuple):
            keys1 = next(zip(*obj1)) if len(obj1) != 0 else []
            keys2 = next(zip(*obj2)) if len(obj2) != 0 else []
            if keys1 != keys2:
                return ["Different keys in %s object: expected: %s%s%s vs actual: %s%s%s"
                        % (parent, escapes['passed_arg'], keys1, escapes['reset'], escapes['failed_arg'], keys2,
                           escapes['reset'])]
            for i in range(0, len(keys1)):
                problems += diff_recursive(parent + "[" + keys1[i] + "]", obj1[i][1], obj2[i][1])
            return problems

        if len(obj1) != len(obj2):
            return ["Different count of elements in %s array: Expected: %s%s%s vs Actual: %s%s%s"
                    % (parent, escapes['passed_arg'], len(obj1), escapes['reset'], escapes['failed_arg'], len(obj2),
                       escapes['reset'])]
        for i in range(0, len(obj1)):
            problems += diff_recursive(parent + "[" + str(i) + "]", obj1[i], obj2[i])
        return problems

    else:
        # The pattern is the second argument of fnmatch
        if not fnmatch.fnmatch(str(obj2), str(obj1)):
            return ["Different values for %s: Expected: '%s%s%s' vs Actual: '%s%s%s'"
                    % (parent, escapes['passed_arg'], obj1, escapes['reset'], escapes['failed_arg'],
                       obj2, escapes['reset'])]
        return problems


def diff_json_pattern_values(parent, obj1, obj2):
    """
    Recursively compare json objects obj1 and obj2
    String values of obj1 can contain fnmatch patterns

    Returns a list with problems
    """

    # Recursively sort lists
    # (and convert dictionaries to lists of (key, value) pairs so that they're orderable)
    def ordered(obj):
        if isinstance(obj, list):
            return sorted(ordered(x) for x in obj)
        if isinstance(obj, dict):
            return sorted((k, ordered(v)) for k, v in obj.items())
        else:
            return obj

    try:
        obj1 = ordered(obj1)
    except TypeError:
        raise AssertionError("Cannot sort expected json, this could be caused by different types in an array.")

    try:
        obj2 = ordered(obj2)
    except TypeError:
        raise AssertionError("Cannot sort input json, this could be caused by different types in an array.")

    return diff_recursive(parent, obj1, obj2)
