# Commit Message Standards

This page explains the shift from the old Jira-based commit style to the new GitHub issue-closing style.

## Previous Style

```text
HPCC-12345 Add support for ...
```

## New Standard

Use one of these lead verbs:
- `Fixes`
- `Resolves`
- `Closes`

Format:

```text
Fixes #12345: Add support for ...
```

## Why This Matters

GitHub uses `Fixes`, `Resolves`, and `Closes` to automatically close linked issues when a pull request is merged.

## Best Practices

- Use one issue reference per commit when practical.
- Keep the summary concise and imperative.
- For multi-commit pull requests, ensure at least one commit or the PR description includes the closing keyword.
- Use the same issue number in the branch name and the commit message when possible.

## Examples

- `Fixes #4821: Add WU query filter`
- `Resolves #4902: Fix Dali lock timeout`
- `Closes #5010: Update enterprise migration guide`

## See also

- [Home](Home)
- [Enterprise Migration Overview](Enterprise%20Migration%20Overview)
- [Daily Developer Workflow](Daily%20Developer%20Workflow)
