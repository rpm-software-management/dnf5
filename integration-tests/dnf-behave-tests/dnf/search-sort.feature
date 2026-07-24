Feature: Sort search command output


Background:
  Given I use repository "search-sort"


@bz1811802
Scenario: sort alphanumerically
  When I execute dnf with args "search name"
  Then the exit code is 0
   And stderr is
       """
       <REPOSYNC>
       """
   And stdout is
       """
       Matched fields: name (exact)
        name.src	Summary
        name.x86_64	Summary
       Matched fields: name, summary
        name-summary.src	Summary of name
        name-summary.x86_64	Summary of name
        name-summary-description.src	Summary of name
        name-summary-description.x86_64	Summary of name
        name-summary-description-url.src	Summary of name
        name-summary-description-url.x86_64	Summary of name
        name-summary-url.src	Summary of name
        name-summary-url.x86_64	Summary of name
       Matched fields: name
        name-description.src	Summary
        name-description.x86_64	Summary
        name-description-url.src	Summary
        name-description-url.x86_64	Summary
        name-url.src	Summary
        name-url.x86_64	Summary
       Matched fields: summary
        summary.src	Summary of name
        summary.x86_64	Summary of name
        summary-description.src	Summary of name
        summary-description.x86_64	Summary of name
        summary-description-url.src	Summary of name
        summary-description-url.x86_64	Summary of name
        summary-url.src	Summary of name
        summary-url.x86_64	Summary of name
       """


@bz1811802
Scenario: sort --all alphanumerically
  When I execute dnf with args "search --all name"
  Then the exit code is 0
   And stderr is
       """
       <REPOSYNC>
       """
   And stdout is
       """
       Matched fields: name (exact)
        name.src	Summary
        name.x86_64	Summary
       Matched fields: name, summary, description, url
        name-summary-description-url.src	Summary of name
        name-summary-description-url.x86_64	Summary of name
       Matched fields: name, summary, description
        name-summary-description.src	Summary of name
        name-summary-description.x86_64	Summary of name
       Matched fields: name, summary, url
        name-summary-url.src	Summary of name
        name-summary-url.x86_64	Summary of name
       Matched fields: name, summary
        name-summary.src	Summary of name
        name-summary.x86_64	Summary of name
       Matched fields: name, description, url
        name-description-url.src	Summary
        name-description-url.x86_64	Summary
       Matched fields: name, description
        name-description.src	Summary
        name-description.x86_64	Summary
       Matched fields: name, url
        name-url.src	Summary
        name-url.x86_64	Summary
       Matched fields: summary, description, url
        summary-description-url.src	Summary of name
        summary-description-url.x86_64	Summary of name
       Matched fields: summary, description
        summary-description.src	Summary of name
        summary-description.x86_64	Summary of name
       Matched fields: summary, url
        summary-url.src	Summary of name
        summary-url.x86_64	Summary of name
       Matched fields: summary
        summary.src	Summary of name
        summary.x86_64	Summary of name
       Matched fields: description
        description.src	Summary
        description.x86_64	Summary
       Matched fields: url
        url.src	Summary
        url.x86_64	Summary
       """
