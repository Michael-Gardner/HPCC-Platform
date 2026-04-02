# GitHub CLI and Authentication

Use the GitHub CLI in WSL2 so authentication and account switching are available directly in your Linux shell workflow.

## Install gh in WSL2

### Ubuntu/Debian-based WSL2 install

```bash
sudo apt update
sudo apt install -y gh
```

If your package feed does not include a recent `gh`, add the official GitHub CLI package repository and install from there.

### Verify installation

```bash
gh --version
```

## Authenticate with Browser Login

During migration, many developers will temporarily need both a personal/open-source identity and an enterprise EMU identity.

From WSL2:

```bash
gh auth login --web --hostname github.com --git-protocol https
```

When prompted:
- Copy the one-time code displayed in the terminal.
- Press `Enter` to open `https://github.com/login/device` in your browser.
- Sign in with your EMU account and enter the one-time code.

Repeat login as needed for additional accounts.

## Check Authentication Status

```bash
gh auth status
```

This confirms:
- Which account is active
- Which hosts are authenticated
- Token validity and scopes

## Switch Accounts

```bash
gh auth switch
```

Use this when moving between personal and enterprise work. Always verify with `gh auth status` before `fetch`, `pull`, or `push` against the enterprise remote.

## Remove Old Credentials After Migration

Once your local repository uses a single enterprise remote and you no longer need old credentials:

```bash
gh auth logout --hostname github.com
gh auth status
```

Only remove credentials for accounts you no longer need for this repository.

## See also

- [Home](Home)
- [Repository Remotes and Branching](Repository%20Remotes%20and%20Branching)
- [Daily Developer Workflow](Daily%20Developer%20Workflow)
