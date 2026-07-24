# -*- coding: utf-8 -*-

from __future__ import absolute_import
from __future__ import print_function

import operator

from behave.tag_matcher import ActiveTagMatcher


class VersionedActiveTagMatcher(ActiveTagMatcher):
    @staticmethod
    def version_compare_operator(tag_value_str, current_value_str, is_negated=False):
        tag_key, tag_oper, tag_value = tag_value_str.split("__", 2)
        try:
            current_key, current_value = current_value_str.split("__", 1)
        except ValueError:
            return is_negated

        # convert versions into list of integers for correct comparing
        tag_value = [int(i) for i in tag_value.split(".")]
        try:
            current_value = [int(i) for i in current_value.split(".")]
        except:
            return is_negated

        if tag_oper not in ["lt", "le", "eq", "ne", "ge", "gt"]:
            raise ValueError("Invalid operator in tag: %s" % tag_value_str)

        if tag_key != current_key:
            # always return false if keys do not match
            return is_negated

        op = getattr(operator, tag_oper)
        result = op(current_value, tag_value)
        if is_negated:
            result = not (result)
        return result

    def is_tag_group_enabled(self, group_category, group_tag_pairs):
        """Modified ActiveTagMatcher.is_tag_group_enabled()"""
        if not group_tag_pairs:
            # -- CASE: Empty group is always enabled (CORNER-CASE).
            return True

        current_value = self.value_provider.get(group_category, None)
        if current_value is None and self.ignore_unknown_categories:
            # -- CASE: Unknown category, ignore it.
            return True

        enabled_positive_tags = []
        # Negative tags are those with the `not.` prefix.
        enabled_negative_tags = []
        for category_tag, tag_match in group_tag_pairs:
            tag_prefix = tag_match.group("prefix")
            category = tag_match.group("category")
            tag_value = tag_match.group("value")
            assert category == group_category
            is_tag_negated = self.is_tag_negated(tag_prefix)
            tag_enabled = self.version_compare_operator(tag_value, current_value, is_tag_negated)
            if is_tag_negated:
                enabled_negative_tags.append(tag_enabled)
            else:
                enabled_positive_tags.append(tag_enabled)

        # LOGICAL OR: There must be either no positive tags, or at least one of them must match.
        positive_tags_satisfied = (not enabled_positive_tags) or any(enabled_positive_tags)

        # LOGICAL AND: There must be either no negative tags, or all of them must match.
        # For example, to safisfy:
        #   @not.with_os=rhel__ge__9
        #   @not.with_os=fedora__ge__39
        # rhel must not be >= 9 AND fedora must not be >= 39.
        negative_tags_satisfied = (not enabled_negative_tags) or all(enabled_negative_tags)

        return positive_tags_satisfied and negative_tags_satisfied
