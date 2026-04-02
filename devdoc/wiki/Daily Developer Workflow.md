# Daily Developer Workflow

This page describes the standard workflow for creating a branch, committing work, pushing to the enterprise repository, and opening a pull request.

## 1. Switch to Enterprise Identity and Create a Branch from a Candidate Base

```bash
gh auth switch
gh auth status

git fetch enterprise
git checkout --track -b feature/<username>/<issue number>-<description> enterprise/candidate-{major}.{minor}.x
```

## 2. Implement and Commit Locally

Use `Fixes|Resolves|Closes #<issue number>:` in the commit message so the linked GitHub issue closes automatically when the pull request is merged.

```bash
git add <files>
git commit -m "Fixes #<issue number>: Short Description"
```

## 3. Re-Verify gh Account Before Push

This is mainly needed during the multi-remote transition period.

```bash
gh auth switch
gh auth status
```

Once your repository uses a single `origin` remote pointing to `https://github.com/risk-hsy/HPCC-Platform`, you can usually skip this extra step.

## 4. Push to the Enterprise Repository

```bash
git push -u enterprise feature/<username>/<issue number>-<description>
```

## 5. Open the Pull Request

CLI option:

```bash
gh pr create --base candidate-{major}.{minor}.x --head feature/<username>/<issue number>-<description> --fill
```

Browser option:
- Go to `risk-hsy/HPCC-Platform`
- Switch to your branch
- Click `Contribute`
- Complete the pull request form as normal

## 6. Remove Unused Credentials After Migration

After simplifying to a single enterprise remote, you can remove old CLI credentials:

```bash
gh auth logout --hostname github.com
gh auth status
```

Only remove credentials for accounts you no longer need.

## See also

- [Home](Home)
- [Repository Remotes and Branching](Repository%20Remotes%20and%20Branching)
- [Commit Message Standards](Commit%20Message%20Standards)
