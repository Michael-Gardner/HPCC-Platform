# Repository Remotes and Branching

This page covers local remote setup, branch naming, and the post-migration cleanup path to a single enterprise remote.

## Typical Remote Setup During Migration

```bash
git remote -v
```

Typical target outcome:
- `upstream` points to OSS upstream: `https://github.com/hpcc-systems/HPCC-Platform.git`
- `origin` points to personal fork: `https://github.com/<github_username>/HPCC-Platform.git`
- `enterprise` points to enterprise shared repo: `https://github.com/risk-hsy/HPCC-Platform.git`

Most developers already have `upstream` and `origin` configured. In that case, only add `enterprise`:

```bash
git remote add enterprise https://github.com/risk-hsy/HPCC-Platform.git
```

Before any enterprise `fetch`, `pull`, or `push`, verify you are on the correct account:

```bash
gh auth status
```

If the active account is not your enterprise identity, switch it first:

```bash
gh auth switch
```

## Branch Naming Standard

All new branches should follow one of these patterns:

- `feature/<developer username>/<issue number>-<description>`
- `bug-fix/<developer username>/<issue number>-<description>`
- `documentation/<developer username>/<issue number>-<description>`

Examples:
- `feature/jdoe/4821-add-wu-query-filter`
- `bug-fix/asmith/4902-fix-dali-lock-timeout`
- `documentation/mchan/5010-update-enterprise-migration-guide`

Use lowercase and hyphen-separated descriptions, and include the issue number in every branch name.

## Candidate Branch Targets

Most work is based on a feature branch with the pattern `candidate-{major}.{minor}.x`, depending on whether the work is a security fix, bug fix, or new feature.

Example branch creation:

```bash
git fetch enterprise
git checkout --track -b feature/<username>/<issue number>-<description> enterprise/candidate-{major}.{minor}.x
```

## Post-Migration Remote Simplification

Once active work has been moved to enterprise-hosted branches, you can simplify your local repository to one remote.

Target end state:
- Remove `upstream` and `origin`
- Rename `enterprise` to `origin`
- Continue normal workflows with a single `origin` remote

Suggested sequence:

```bash
git remote -v
git remote remove upstream
git remote remove origin
git remote rename enterprise origin
git remote -v
```

After this transition, use `origin` as your only remote for `fetch`, `pull`, and `push`.

## See also

- [Home](Home)
- [GitHub CLI and Authentication](GitHub%20CLI%20and%20Authentication)
- [Daily Developer Workflow](Daily%20Developer%20Workflow)
