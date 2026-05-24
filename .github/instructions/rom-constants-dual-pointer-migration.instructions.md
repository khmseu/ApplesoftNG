---
description: "Use when migrating existing ROM constant APIs to canonical label-named ApplesoftDualPointer functions with staged compatibility and low churn."
name: "ROM Constants Dual-Pointer Migration Sequencing"
applyTo: "src/core/**/*.cpp,include/core/**/*.hpp"
---

# Migration Sequencing For ROM Constant APIs

Use these instructions with the primary ROM constants dual-pointer policy to transition existing code safely.

## Staged Migration Order

1. Introduce canonical label-based `ApplesoftDualPointer<T>` function.
1. Add or retain compatibility wrappers preserving existing return types.
1. Update high-leverage consumers to accept dual-pointer input and decoded/native input.
1. Expand caller migration incrementally, preserving behavior at each step.
1. Remove obsolete paths only after no active caller depends on them.

## Churn Control

- Keep each change set focused on one table/family at a time.
- Prefer local adapters over broad signature rewrites when possible.
- Avoid cross-subsystem refactors unless required for compile or correctness.

## Compatibility Requirements

- Existing public helper behavior must remain stable during transition unless explicitly changed.
- If behavior changes are required, include clear comments and update nearby docs/maps.
- Treat wrappers as temporary compatibility surfaces; canonical label APIs remain the source of truth.

## Consumer Update Rules

- Where practical, provide call paths that accept both:
  - `ApplesoftDualPointer<T>`
  - legacy decoded/native value types
- Normalize internally at module boundaries so downstream logic remains simple.

## Verification Expectations

- Build must remain green after each incremental step.
- No new direct ROM constant access pattern should bypass canonical label APIs.
- High-impact tables (`AS_ERROR_MESSAGES`, `AS_MATHTBL`, `AS_TOKEN_NAME_TABLE`, `AS_TOKEN_ADDRESS_TABLE`) should be migrated in explicit, reviewable steps.

## Exit Criteria

- Canonical label functions exist for migrated constants.
- Callers in touched areas can interoperate with dual-pointer and compatibility forms.
- Any retained wrappers are intentional, documented, and delegated to canonical implementations.
