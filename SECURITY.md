# Security Policy

## Supported versions

| Version | Supported |
| ------- | --------- |
| 0.9.2   | ✅ active prototype |
| < 0.9.2 | ❌ |

## Reporting a vulnerability

Please **do not** open a public issue for security problems. Use GitHub's
private vulnerability reporting ("Security" tab → "Report a vulnerability") so
the issue can be triaged before disclosure.

Include reproduction steps, the affected version/commit and the device or host
platform. You can expect an initial response within a few days.

## Signing material

The release signing keystore is deliberately **not** part of this repository and
must never be committed. See [`SIGNING.md`](SIGNING.md) for the key-continuity
rules; losing the original key breaks in-place updates for existing installs.
