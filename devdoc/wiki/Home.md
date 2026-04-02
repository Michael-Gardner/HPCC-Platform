# HPCC-Platform Developer Wiki

Welcome to the enterprise development wiki for the **HPCC-Platform** project, hosted at [github.com/risk-hsy/HPCC-Platform](https://github.com/risk-hsy/HPCC-Platform).

This wiki is the internal knowledge base for developers working within the enterprise managed user (EMU) environment. For the upstream open source project and public documentation, visit [hpccsystems.com](https://hpccsystems.com/).

## About the HPCC Systems Platform

[HPCC Systems](https://hpccsystems.com/) is an end-to-end data lake management solution purpose-built for high-speed data engineering. Originally developed by LexisNexis Risk Solutions, the platform has been in active production development for more than 20 years and has been open source for more than 10 years, with thousands of global deployments.

HPCC Systems' core advantage lies in its lightweight, standards-based architecture. It delivers better performance, near real-time results, and full-spectrum operational scale without requiring a massive development team, unnecessary add-ons, or increased processing costs. Developers write in ECL (Enterprise Control Language), a purpose-built language for parallel data processing that is more concise and efficient than general-purpose alternatives.

The platform is cloud-native and runs on Kubernetes, with support for major cloud providers including Azure Kubernetes Service and Amazon Elastic Kubernetes Service. Its main runtime components are **Thor** for large-scale batch processing, **Roxie** for low-latency query delivery, and **ESP** for exposing platform services over standard web protocols.

## This Enterprise Repository

This repository is the **enterprise environment** for internal HPCC Systems platform development.

### Project Leader

- Gavin Halliday

### Maintainer Group

- Gavin Halliday
- Gordon Smith
- Jake Cobbett-Smith

## Repository Access and Branch Permissions

### HPCC-Platform Team members

Any member of the HPCC-Platform Team may:
- Create and push development branches in the repository.
- Open pull requests targeting feature branches such as `candidate-{major}.{minor}.x`.

### Maintainer Group

Only the Maintainer group may:
- Accept and merge pull requests into `candidate-{major}.{minor}.x` branches.
- Merge into any `candidate-*` branch.
- Merge into `master`.

## Wiki Navigation

- [Enterprise Migration Overview](Enterprise%20Migration%20Overview)
- [GitHub CLI and Authentication](GitHub%20CLI%20and%20Authentication)
- [Repository Remotes and Branching](Repository%20Remotes%20and%20Branching)
- [EMU Email Setup](EMU%20Email%20Setup)
- [GPG Signing Setup](GPG%20Signing%20Setup)
- [Daily Developer Workflow](Daily%20Developer%20Workflow)
- [Commit Message Standards](Commit%20Message%20Standards)
- [Permissions and Merge Policy](Permissions%20and%20Merge%20Policy)

## External Resources

- [HPCC Systems Official Site](https://hpccsystems.com/)
- [Public GitHub Repository](https://github.com/hpcc-systems/HPCC-Platform)
- [ECL Playground](https://play.hpccsystems.com:18010/)
- [Official Documentation](https://hpccsystems.com/training/documentation/)
