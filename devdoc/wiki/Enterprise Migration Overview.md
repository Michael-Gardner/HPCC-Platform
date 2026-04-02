# Enterprise Migration Overview

This page explains the shift from the public open source fork-based workflow to the shared repository workflow used in the enterprise environment.

## What Changed

### Previous model

- Developers typically forked the upstream repository.
- Work branches were developed on personal forks.
- Pull requests were opened from fork to upstream.
- Commit messages followed a Jira-style format such as `HPCC-12345 Short Description`.

### New enterprise model

- Developers create branches locally and push them directly to `https://github.com/risk-hsy/HPCC-Platform.git`.
- Pull requests target feature branches such as `candidate-{major}.{minor}.x`.
- Commit messages should use GitHub issue-closing keywords:
  - `Fixes #<issue number>: Short Description`
  - `Resolves #<issue number>: Short Description`
  - `Closes #<issue number>: Short Description`

This commit format is important because GitHub uses these keywords to automatically close linked issues when the pull request is merged.

## Recommended Reading Order

1. [GitHub CLI and Authentication](GitHub%20CLI%20and%20Authentication)
2. [Repository Remotes and Branching](Repository%20Remotes%20and%20Branching)
3. [EMU Email Setup](EMU%20Email%20Setup)
4. [GPG Signing Setup](GPG%20Signing%20Setup)
5. [Daily Developer Workflow](Daily%20Developer%20Workflow)
6. [Commit Message Standards](Commit%20Message%20Standards)
7. [Permissions and Merge Policy](Permissions%20and%20Merge%20Policy)

## Key Repositories

- OSS upstream: `https://github.com/hpcc-systems/HPCC-Platform.git`
- Enterprise shared repo: `https://github.com/risk-hsy/HPCC-Platform.git`

## See also

- [Home](Home)
- [Daily Developer Workflow](Daily%20Developer%20Workflow)
- [Commit Message Standards](Commit%20Message%20Standards)
