# EMU Email Setup

Before configuring signed commits, make sure your EMU account email is correct, unique to your enterprise account, and verified.

## Verify Your EMU Email in the Browser

1. On your managed machine, sign in to your EMU account in the web browser.
2. Open your user icon menu.
3. Go to `Settings` -> `Emails`.
4. Note the email address associated with your EMU account.

## Remove That Email from Your Personal GitHub Account

An email used by your EMU account cannot also be used by another GitHub account.

> **Important:** GitHub requires at least one email address on every account. If the EMU email is the only email on your personal account, you must add a different email address first before you can remove it. A personal email address works fine — the goal is simply to ensure the EMU email is exclusively associated with your enterprise account.

1. In the browser, open your user icon menu and choose `Switch account`.
2. Switch to your personal GitHub account.
3. Go to `Settings` -> `Emails`.
4. If the EMU email is the only email listed, add a personal or alternative email address and verify it first.
5. Once a second email is in place, remove the EMU email from this account.

## Why This Matters

- GitHub commit attribution depends on the commit email matching the correct account.
- Commit verification works best when the EMU email is unique to the enterprise account.
- This avoids identity confusion between personal and enterprise GitHub users.

## Verify the EMU Email Address

After setting the correct EMU email:

1. In your enterprise account, go to `Settings` -> `Emails`.
2. On the right side of the EMU email row, click the `...` menu.
3. Select `Verify email`.
4. Open Outlook and find the verification email from GitHub.
5. Copy the secret verification code from that email.
6. Return to `Settings` -> `Emails` in GitHub.
7. Click the `...` menu again for that email.
8. Select `Input verification code`.
9. Paste the code and press `Enter`.
10. Confirm the email now shows as `Verified`.

## See also

- [Home](Home)
- [GPG Signing Setup](GPG%20Signing%20Setup)
- [Daily Developer Workflow](Daily%20Developer%20Workflow)
