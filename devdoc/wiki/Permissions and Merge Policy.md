# Permissions and Merge Policy

This page summarizes who can write branches and who can approve or merge pull requests in the enterprise repository.

## Repository Roles

### Project Leader

**Gavin Halliday**

### Maintainer Group

- Gavin Halliday
- Gordon Smith
- Jake Cobbett-Smith

## HPCC-Platform Team Members

Any member of the HPCC-Platform Team can:
- Create and push to development branches in the repository.
- Open pull requests targeting `candidate-{major}.{minor}.x` feature branches.

## Maintainer Group Responsibilities

Only members of the Maintainer group can:
- Accept and merge pull requests into `candidate-{major}.{minor}.x` branches.
- Merge into any `candidate-*` branch.
- Merge into `master`.

This ensures shared release and integration branches remain under maintainer control.

## See also

- [Home](Home)
- [Enterprise Migration Overview](Enterprise%20Migration%20Overview)
- [Daily Developer Workflow](Daily%20Developer%20Workflow)
