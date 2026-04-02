# GPG Signing Setup

Use GPG-signed commits in WSL2 so GitHub can verify commit signatures in the enterprise repository.

## Install GPG in WSL2

```bash
sudo apt update
sudo apt install -y gnupg2 pinentry-curses
```

Verify installation:

```bash
gpg --version
```

## Generate a GPG Key for Your EMU Identity

```bash
gpg --full-generate-key
```

Recommended options:
- Key type: `RSA and RSA`
- Key size: `4096`
- Expiration: follow team policy, for example `1y`
- Real name: your enterprise display name
- Email: your EMU email address

List keys and capture the signing key ID:

```bash
gpg --list-secret-keys --keyid-format=long
```

Example:

```text
sec   rsa4096/ABCDEF1234567890 2026-03-31 [SC]
```

In this example, the key ID is `ABCDEF1234567890`.

## Add the Public Key to GitHub

Export the public key:

```bash
gpg --armor --export ABCDEF1234567890
```

Copy the full armored block and add it in GitHub:
- Go to `https://github.com/settings/keys`
- Select `New GPG key`
- Paste the armored public key
- Save

## Configure Git to Sign Commits Automatically

Set repository-local Git config for the enterprise repository:

```bash
git config user.name "<your enterprise name>"
git config user.email "<your EMU email>"
git config user.signingkey ABCDEF1234567890
git config commit.gpgsign true
git config gpg.program gpg
```

Optional:

```bash
git config tag.gpgsign true
```

## Validate Signature Setup

Create a temporary signed commit:

```bash
git commit --allow-empty -S -m "chore: validate GPG signing setup"
git log --show-signature -1
```

If needed, remove the temporary commit:

```bash
git reset --soft HEAD~1
```

## Multi-Account Notes

- Use repo-local Git config for EMU repositories.
- Make sure `user.email` matches an email recognized by your enterprise GitHub account.
- Authentication selection with `gh auth switch` is separate from commit signing.

## See also

- [Home](Home)
- [EMU Email Setup](EMU%20Email%20Setup)
- [Daily Developer Workflow](Daily%20Developer%20Workflow)
