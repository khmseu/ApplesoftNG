---
name: conventional-commits
description: "Use when writing, reviewing, linting, or automating commit messages with the Conventional Commits 1.0.0 specification, including type/scope syntax, breaking change signaling, and SemVer/changelog workflows."
context: fork
---

# Conventional Commits

Use this skill when creating commit message policies, reviewing commit quality, or integrating commit-driven release automation.

This skill is based on official documentation:
- https://www.conventionalcommits.org/en/v1.0.0/
- https://www.conventionalcommits.org/en/about/

## Domain Focus

This skill covers:
- Correct Conventional Commits message structure
- Type, scope, description, body, and footer rules
- Breaking change signaling (`!` and `BREAKING CHANGE:`)
- SemVer implications of commit types
- Practical team workflows for linting and changelog/version automation

## Best Practices

### Message Structure

- Use the canonical structure:

```text
<type>[optional scope]: <description>

[optional body]

[optional footer(s)]
```

- Keep descriptions short, specific, and action-oriented.
- For any non-trivial change, include a detailed body that explains context, rationale, and key implementation details.
- Keep one-line, header-only messages for truly trivial changes only (for example typo-only docs fixes).
- Use footers for metadata (`Refs:`, `Reviewed-by:`) and release signals.

### Types and Scope

- Use `feat` for new features and `fix` for bug fixes.
- Use additional types (`docs`, `refactor`, `perf`, `test`, `chore`, `ci`, `build`, etc.) consistently per team policy.
- Use scope as a noun indicating affected subsystem, for example `feat(parser): ...`.
- Prefer splitting mixed changes into multiple commits rather than forcing one ambiguous type.

### Breaking Changes

- Mark breaking changes with either:
  - `!` in the header (for example `feat(api)!: ...`), or
  - `BREAKING CHANGE: ...` footer.
- Use explicit migration language in the breaking change description.
- When possible, include upgrade steps in commit body/footer for downstream users.

### SemVer and Automation

- Map commit intent to release impact:
  - `fix` -> PATCH
  - `feat` -> MINOR
  - `BREAKING CHANGE` (any type) -> MAJOR
- Keep commit messages machine-parseable to support changelog and version tooling.
- Enforce commit format in CI or hooks when automation depends on it.

## Common Pitfalls

- Missing required colon-space after type/scope.
- Using vague descriptions that do not describe behavior change.
- Marking breaking behavior without `!` or `BREAKING CHANGE:`.
- Combining unrelated fixes/features in a single commit.
- Inconsistent casing/type vocabulary across a repository.
- Treating non-conforming commits as harmless when release tooling depends on parsing.

## Practical Patterns

### Basic Feature and Fix

```text
feat(auth): add OAuth device flow
fix(cache): avoid stale entries after invalidation
```

### Breaking Change in Footer

```text
feat: allow provided config object to extend other configs

BREAKING CHANGE: `extends` now references external config files.
```

### Breaking Change via Header Marker

```text
feat(api)!: remove legacy v1 response shape
```

### Body and Multiple Footers

```text
fix: prevent racing of requests

Introduce request IDs and discard late responses from superseded requests.
This replaces timeout-based mitigation with deterministic latest-request wins behavior.
Adds regression coverage for out-of-order network responses.

Reviewed-by: Z
Refs: #123
```

### Revert Pattern

```text
revert: undo parser fallback behavior

Refs: 676104e, a215868
```

## Review Checklist

- Does the header follow `<type>[scope]: <description>` exactly?
- Is the selected type accurate for the change intent?
- If scope is used, is it meaningful and consistent?
- Is breaking behavior clearly marked and explained?
- Does the body provide necessary rationale or migration context?
- For non-trivial changes, is the body detailed enough to explain why and how the change was made?
- Are footer tokens valid and useful for tooling/humans?
- Will this message produce correct changelog and SemVer behavior?

## Output Expectations

When helping with Conventional Commits tasks, prefer guidance that:
- preserves strict parseability for tooling
- aligns type semantics with actual change impact
- makes breaking changes explicit and migration-friendly
- encourages small, single-purpose commits for clear release automation
- defaults to detailed commit bodies for non-trivial changes, while allowing concise headers for trivial changes
