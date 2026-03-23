# Shared Repository Model

This guide explains how to use a shared repository model in **HPCC-Platform** where contributors work in the same repository, while branch rulesets protect release-critical branches.

## Purpose

The shared repository model is designed to:

- Keep collaboration fast by allowing contributors to create branches in the main repository.
- Preserve release quality by preventing direct writes to protected branches.
- Ensure all important changes are reviewed, tested, and traceable through pull requests.

## Why Shared Repository Instead of Forked Model?

Both models are valid, but for HPCC-Platform's day-to-day team development, a shared repository model is typically more effective.

### Shared Repository Model (preferred for HPCC-Platform)

- **Single source of truth for active work**: all feature branches live in one repository, making discovery and collaboration easier.
- **Faster cross-team iteration**: reviewers and teammates can push follow-up commits to a contributor branch (when allowed by team process) without cross-fork permission friction.
- **Simpler CI and branch governance**: one set of rulesets, required checks, and branch protections applies consistently.
- **Lower operational overhead**: no need to keep many personal forks synchronized with upstream before every new change.
- **Better release discipline with rulesets**: strict controls on `master` and `candidate-*` coexist with flexible contributor branches.

### Forked Model (useful in some scenarios)

Fork-based workflows are often better when:

- Contributions come from a large external/open-source community with unknown trust levels.
- Repository write access should be highly restricted.
- Teams intentionally want strict isolation between contributor branches and the main repository.

### Why HPCC-Platform Chooses Shared

For an internal or tightly coordinated contributor group, shared-repo workflow improves velocity while preserving safety through branch rulesets. In practice, HPCC-Platform gets:

- collaborative development in one place,
- protected integration branches (`master`, `candidate-*`), and
- clear review, CI, and merge controls without fork-management overhead.

### Why Shared Model Fits Enterprise Environments

Open-source projects often prefer forks because contributors are external and trust boundaries are broad. Enterprise environments are different: contributors are usually known employees or approved contractors, and governance is centered on controlled in-repo collaboration.

In enterprise settings, the shared model works well because it provides:

- **Centralized identity and access control**: permissions are managed in one repository using teams/roles, not duplicated across many personal forks.
- **Stronger compliance and auditability**: approvals, required checks, and branch-protection events are captured in a single governance surface.
- **Consistent CI/CD and secret handling**: one pipeline policy and one secure secret boundary reduce operational risk and configuration drift.
- **Clear release management**: maintainers can enforce strict controls on `master` and `candidate-*` while still allowing fast contributor iteration on feature branches.
- **Lower support overhead**: no need to troubleshoot stale forks, missing upstream syncs, or divergent fork settings across many users.
- **Better cross-team ownership**: branches are visible and accessible to the team, which improves handoffs, pairing, and continuity during PTO or team changes.

### Model Decision Matrix

| Decision Area | Shared Repository Model | Forked Model | Better Fit for Enterprise HPCC-Platform |
|---|---|---|---|
| Contributor trust boundary | Known internal users (employees/approved contractors) | Unknown or broad external contributor base | Shared |
| Access control | Centralized in one repo (teams/roles/rulesets) | Distributed across many forks + upstream PR permissions | Shared |
| Branch governance | Uniform rulesets for `master`, `candidate-*`, and contributor branches | Harder to enforce consistent branch policy across forks | Shared |
| CI/CD consistency | One pipeline policy, one required-check baseline | Possible drift between fork and upstream settings/workflows | Shared |
| Secret management | Single controlled secret boundary | More complex with external fork contexts | Shared |
| Compliance/audit trail | Centralized approvals, checks, and merge events | More fragmented trail across multiple repos | Shared |
| Operational overhead | Low (no fork sync management) | Higher (syncing stale forks, branch drift) | Shared |
| Cross-team collaboration | Easy branch visibility and shared ownership | Slower, extra permissions and cross-fork friction | Shared |
| External community intake | Less ideal at very large open contribution scale | Strong fit for large OSS ecosystems | Forked |
| Recommended default for HPCC-Platform | Yes | Only for specific external-contributor scenarios | Shared |

## Repository Roles

Use two practical role tiers for day-to-day work:

- **Collaborators**
	- Can write to the repository and create normal feature branches.
	- Must use pull requests for integration into protected branches.
	- Should not push directly to `master` or `candidate-*` branches.
- **Maintainers**
	- Can perform release management tasks.
	- Can write to protected branches when needed for approved release operations.
	- Own final merge and stabilization decisions.

## Branch Strategy

Recommended branch categories for HPCC-Platform:

- **Protected integration branches**
	- `master`
	- `candidate-*`
- **Contributor feature branches**
	- `feature/<issue-number>-<short-description>`
	- `bugfix/<issue-number>-<short-description>`
	- `docs/<issue-number>-<short-description>`

Contributors develop on feature branches, open pull requests, and let maintainers control integration into protected branches.

### Branch Naming: Why Not `<username>-<issue>-<description>`?

A common impulse is to name branches `<username>-<issue>-<description>` (for example, `michael-1234-fix-roxie-crash`). While intuitive, this pattern has several drawbacks in a shared repository:

- **Type context is lost** — reviewers cannot distinguish a feature branch from a bugfix or documentation change at a glance. Triage and bulk filtering become harder.
- **Ruleset pattern matching breaks** — GitHub branch rulesets use glob patterns (`feature/**`, `bugfix/**`) to apply different rules per branch category. A flat `<username>-*` namespace collapses all branch types into one undifferentiated group.
- **Authorship is already recorded** — git commit history and the GitHub PR record who created the branch. The username in the branch name is redundant.
- **Signals personal ownership** — it discourages collaborators from adding commits to a branch in progress and works against the cooperative intent of the shared repository model.
- **Inconsistent with HPCC-Platform's tracker** — HPCC-Platform uses GitHub Issues. A flat `<username>-*` prefix obscures the issue number and branch type simultaneously.

**Acceptable compromise (optional):** If author visibility is important to your team, a sub-path layout preserves type prefix while surfacing the author:

```
feature/<username>/1234-short-description
bugfix/<username>/1234-short-description
```

This keeps ruleset patterns working (`feature/**`, `bugfix/**`) and makes the branch owner visible without abandoning branch type context.

## Branch Rulesets

Configure GitHub **Repository Rulesets** to enforce behavior. Use a dedicated ruleset for protected branches.

### Ruleset A: Protected Branches (`master`, `candidate-*`)

**Target branches**

- `master`
- `candidate-*`

**Enforcement (recommended)**

- Restrict direct pushes.
- Require pull requests before merging.
- Require at least one approving review.
- Require status checks to pass.
- Require branch to be up to date before merging.
- Block force pushes.
- Block branch deletion.

**Bypass / write permissions**

- Allow bypass only for the **Maintainers** team (or equivalent role mapping).
- Do **not** include general collaborators in bypass lists.

This setup satisfies the policy: collaborators can write to the repository, but only maintainers can write directly to `master` and `candidate-*`.

### Ruleset B: Contributor Branches (optional hardening)

You may add a second ruleset for contributor branches (for example, `feature/**`, `bugfix/**`, `docs/**`) to prevent accidental force-push or branch deletion if your team prefers stricter controls.

### GitHub Settings Click Path (Maintainers)

Use this exact click path to configure secure rulesets in GitHub:

1. Open the repository: **HPCC-Platform**.
2. Click **Settings**.
3. In the left sidebar, click **Rules**.
4. Click **Rulesets**.
5. Click **New ruleset** → **New branch ruleset**.

#### Configure Ruleset A (Protected Branches)

Set the following values:

- **Ruleset name**: `Protected Integration Branches`
- **Enforcement status**: `Active`
- **Target**: `Branches`
- **Include by pattern**:
	- `master`
	- `candidate-*`

Enable these rules:

- **Restrict updates** (prevents general direct writes)
- **Require a pull request before merging**
	- Required approvals: at least `1`
	- Dismiss stale approvals when new commits are pushed (recommended)
- **Require status checks to pass**
	- Select required checks used by HPCC-Platform CI
- **Require branches to be up to date before merging**
- **Block force pushes**
- **Block deletions**

Set bypass access:

1. Find the **Bypass list** (or **Allow specified actors to bypass**).
2. Add only the `Maintainers` team (or equivalent maintainer-only group).
3. Confirm no collaborator-level team or broad role is in bypass.

Save the ruleset.

#### Configure Ruleset B (Optional Contributor Branch Hardening)

Create a second branch ruleset with:

- **Ruleset name**: `Contributor Branch Guardrails`
- **Enforcement status**: `Active`
- **Target**: `Branches`
- **Include by pattern**:
	- `feature/**`
	- `bugfix/**`
	- `docs/**`

Recommended rules:

- Do **not** block force pushes if your team expects contributors to run interactive rebase/squash before merge.
- Block deletion (optional, based on cleanup policy).

Interactive rebase rewrites branch history, so contributors must be able to force-push their own `feature/**`, `bugfix/**`, and `docs/**` branches after review updates.

Recommended policy for this workflow:

- Keep force-push blocked on protected branches (`master`, `candidate-*`).
- Allow force-push on contributor branches used for active PRs.
- Require final review/sign-off after the squash/rebase push.

If your team prefers never rewriting branch history, then block force-push on contributor branches and use GitHub's **Squash and merge** at merge time instead.

#### Verification Checklist

After saving rulesets, verify behavior immediately:

1. Using a collaborator account, attempt a direct push to `master` (should be denied).
2. Using a collaborator account, open a PR to `master` (should be allowed).
3. Ensure merge is blocked until required review/checks are complete.
4. Confirm only maintainers can bypass protections for emergency operations.
5. Repeat the direct push test on a `candidate-*` branch.

If any test fails, review pattern matching and bypass actor assignments first.

## Pull Request Workflow

1. Create a branch from the appropriate base (usually `master`, unless instructed otherwise).
2. Commit with clear messages that reference the GitHub issue number (for example, `#1234`).
3. Open a pull request into `master` or the correct `candidate-*` branch.
4. Ensure all required checks pass.
5. Obtain required approvals.
6. Maintainer merges according to release policy.

## Branch Cleanup After Merge

Contributor branches should be short-lived. After a PR is merged, the branch should be deleted to keep the repository tidy.

Recommended cleanup approach:

- Enable **Settings → General → Pull Requests → Automatically delete head branches**.
- Keep long-lived branches (`master`, `candidate-*`) protected and never treated as cleanup targets.
- If needed, delete merged branches manually from the PR page using **Delete branch**.

Local cleanup for contributors:

```bash
git fetch --prune
git branch -d <branch-name>
```

Use `git branch -D <branch-name>` only when a branch is not fully merged locally but is no longer needed.

## Example: Day-to-Day Development

- A collaborator creates `feature/1234-improve-xyz` (where `1234` is the GitHub issue number).
- The collaborator pushes commits and opens a PR to `master`.
- GitHub ruleset blocks direct push to `master` and requires review/checks.
- After approval and green checks, a maintainer merges.
- For release stabilization, maintainers cherry-pick or merge into `candidate-*` under the same protections.

## Operational Guidance for Maintainers

- Keep bypass use rare and auditable.
- Prefer normal PR merges even when bypass is technically available.
- Use direct writes only for urgent operational scenarios (for example, release hotfix handling), and document rationale in PR or issue history.

## Common Pitfalls

- Giving collaborators bypass permissions on protected branches.
- Allowing direct pushes to `candidate-*` during stabilization.
- Disabling required checks for convenience.
- Skipping review on high-impact changes.

## Summary

In HPCC-Platform, the shared repository model works best when contributors are empowered to work in-repo, while `master` and `candidate-*` remain maintainer-controlled through GitHub branch rulesets. This balances development velocity with release safety.
