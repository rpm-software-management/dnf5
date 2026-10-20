# Copyright Contributors to the DNF5 project.
# SPDX-License-Identifier: LGPL-2.1-or-later
"""Render cross-file section references as manual citations in man output.

HTML anchors do not identify the destination manual in man output. This
extension lets documentation use a single :ref: link for both formats: HTML
keeps the hyperlink, while man output keeps the caption and appends the
destination's manual name and section from Sphinx's man_pages configuration.
For example, a link captioned "Vendor change policies" becomes
"Vendor change policies (dnf5.conf-vendorpolicy(5))".

The transform runs before Sphinx's reference resolver and uses the standard
domain to resolve labels and captions. References within the same document,
references to documents without a configured manual, and unresolved references
are left for Sphinx to handle normally. Other output formats are unaffected.
"""

from docutils import nodes
from sphinx import addnodes
from sphinx.transforms.post_transforms import SphinxPostTransform


class ManpageReferences(SphinxPostTransform):
    # Handle pending references before Sphinx's ReferencesResolver (priority 10).
    default_priority = 5
    formats = ("man",)

    def run(self, **kwargs):
        manuals = {
            docname: (name, str(section))
            for docname, name, description, authors, section in self.config.man_pages
        }
        domain = self.env.get_domain("std")

        for node in list(self.document.traverse(addnodes.pending_xref)):
            if node.get("refdomain") != "std" or node.get("reftype") != "ref":
                continue

            labels = domain.data["anonlabels" if node.get("refexplicit") else "labels"]
            target = labels.get(node["reftarget"])
            if target is None:
                # Let Sphinx report unresolved references as usual.
                continue

            docname = target[0]
            fromdocname = node.get("refdoc", self.env.docname)
            if docname == fromdocname or docname not in manuals:
                continue

            reference = domain.resolve_xref(
                self.env, fromdocname, self.app.builder, "ref",
                node["reftarget"], node, node[0].deepcopy(),
            )
            if reference is None:
                continue

            name, section = manuals[docname]
            citation = f"{name}({section})"

            replacement = list(reference.children)
            replacement.extend([
                nodes.Text(" ("),
                addnodes.manpage(
                    "", citation, page=name, section=section,
                    classes=["manpage"],
                ),
                nodes.Text(")"),
            ])
            node.replace_self(replacement)


def setup(app):
    app.add_post_transform(ManpageReferences)
    return {
        "version": "1.0",
        "parallel_read_safe": True,
        "parallel_write_safe": True,
    }
