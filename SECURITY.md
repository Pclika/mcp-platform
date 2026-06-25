# Security Policy

## Scope

This project includes:

- embedded firmware foundations
- MCP bridge tooling
- examples and prompts
- hardware platform documentation

Security issues may affect:

- device credentials
- update mechanisms
- wireless configuration
- MCP tool exposure
- logs and diagnostics

## Reporting a Vulnerability

Until a public reporting channel is announced, please do not open a public issue for sensitive vulnerabilities.

Instead, report security concerns privately through the maintainer contact path defined in [SUPPORT.md](SUPPORT.md).

## What to Include

- affected component
- affected platform family
- reproduction steps
- expected impact
- whether credentials, update paths, or remote control are involved

## Security Priorities

The highest-priority classes for this project are:

- credential leakage
- unsafe remote control surfaces
- insecure provisioning flows
- weak firmware integrity paths
- unsafe default wireless behavior

