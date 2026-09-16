# Security

This is a pure computation library with no I/O, no network access and no
dynamic allocation after initialization. The realistic bug classes are memory
safety issues in the unchecked entry points when given invalid input (which is
documented as undefined behavior) and incorrect ranks (which are correctness
bugs, not security bugs).

If you find something you believe is a security issue, please report it
privately through
[GitHub's private vulnerability reporting](https://github.com/svlocks/poker/security/advisories/new)
rather than a public issue. You should hear back within a week.

Only the latest release and `main` are supported.
