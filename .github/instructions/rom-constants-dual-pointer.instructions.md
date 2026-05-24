---
description: "Use when porting or refactoring ROM constants and constant-label data; enforce label-named ApplesoftDualPointer canonical access with compatibility wrappers where needed."
name: "ROM Constants Dual-Pointer Policy"
applyTo: "src/core/**/*.cpp,include/core/**/*.hpp"
---

# ROM Constants As Dual-Pointer Canonical APIs

Use these instructions when implementing or refactoring ROM constant data that is represented by assembler labels.

## Canonical Representation

- For each ROM constant label, define one canonical function named after that label.
- The canonical function must return an appropriate `ApplesoftDualPointer<T>` specialization.
- Canonical functions should point at immutable constant data.
- If type or overload ambiguity requires it, a minimal suffix is allowed. Keep the original label clearly visible (for example `AS_ERROR_MESSAGES_ptr`).

## Input Interoperability

- Functions that consume this data must be able to handle both forms on input when practical:
  - `ApplesoftDualPointer<T>`
  - existing decoded/native form (for example `std::string_view`, table-entry structs, function pointers)
- Implement interoperability with overloads, adapters, or internal normalization helpers.
- Compatibility wrappers are allowed: existing decoded-return helpers may remain if they delegate to canonical label-based dual-pointer functions.

## Immediate Enforcement Scope

- Apply this policy immediately for relevant touched code in `src/core` and `include/core`.
- Do not create new ROM constant APIs that bypass canonical label-based dual-pointer functions.

## Safety And Boundary Rules

- Preserve `ApplesoftDualPointer` safety checks and memory-model boundaries.
- Do not bypass `ApplesoftVariables` / `IOPorts` access rules to force pointer behavior.
- Do not introduce runtime coupling to SourceMaterial files.

## Const-Correctness Guidance

- Prefer const-correct pointer specializations for ROM data (recommendation).
- If a non-const specialization is used for compatibility, document the reason near the declaration.

## Hotspot Targets

Prioritize consistent treatment for these tables and related callers:

- `AS_ERROR_MESSAGES`
- `AS_MATHTBL`
- `AS_TOKEN_NAME_TABLE`
- `AS_TOKEN_ADDRESS_TABLE`

## Good Patterns

- Canonical label function returns dual-pointer; wrapper decodes from canonical source.
- Callers accept both dual-pointer and decoded/native forms when practical.
- Minimal naming normalization only when required for valid, unambiguous C++ APIs.

## Avoid

- Returning only decoded/native data from new label APIs with no canonical dual-pointer source.
- Introducing parallel, unrelated constants detached from assembler label naming.
- Breaking existing callers without a compatibility path.
