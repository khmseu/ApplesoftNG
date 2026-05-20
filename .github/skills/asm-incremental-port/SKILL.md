---
name: asm-incremental-port
description: Incrementally port Applesoft assembly label ranges to C++ using an inclusive start label and exclusive end label.
---

# asm-incremental-port

## Purpose

Incrementally port Applesoft historical assembler ranges into C++ using a strict two-label conversion window.

Normative conversion behavior is defined in [.github/instructions/asm-to-cpp.instructions.md](../../instructions/asm-to-cpp.instructions.md).
For function-scoped `AS_Labels` claim formatting and validation, follow [.github/skills/writing-claims/SKILL.md](../writing-claims/SKILL.md).

## Trigger Phrases

- "convert labels X to Y"
- "port asm range"
- "incremental assembler to c++"
- "translate this listing window"

If `start_label` or `end_label` cannot be resolved in the authoritative listing, return an error message specifying which label could not be found.

## Inputs

- `start_label` (inclusive)
- `end_label` (exclusive)

Optional:

- preferred destination subsystem (`core` or `platform`)

## Procedure

### Label Resolution & Analysis

1. Resolve both labels in the authoritative listing under [SourceMaterial/Combo/asrom.lst](../../../SourceMaterial/Combo/asrom.lst).
2. If either label is missing, return an error naming the missing label and stop.
3. Read the bounded range from `start_label` up to but not including `end_label`, including inline comments and comments immediately preceding the range.
4. Consult [docs/function-cross-reference.md](../../../docs/function-cross-reference.md) to identify existing implementations, stub placeholders, and source locations in this window.

### Implementation

1. Summarize intent in exactly 3-5 bullets, each limited to a maximum of 15 words, before coding.
2. Implement the conversion using [.github/instructions/asm-to-cpp.instructions.md](../../instructions/asm-to-cpp.instructions.md) as the authoritative rule set.
3. Ensure full `start_label`..`end_label` coverage, including split-into-multiple-functions behavior when needed.

### Finalization

1. Add missing dependency stubs near the new implementation with `TODO(asm-port)` markers.
2. Add or update function-scoped `AS_Labels` claim comments per [.github/skills/writing-claims/SKILL.md](../writing-claims/SKILL.md).
3. Update [docs/function-cross-reference.md](../../../docs/function-cross-reference.md) to reflect newly ported functions and updated stub/real status.
4. Build and report exact files changed.

## Definition Of Done

All items in the Output Checklist from [.github/instructions/asm-to-cpp.instructions.md](../../instructions/asm-to-cpp.instructions.md) are satisfied.

## Notes

- Prefer small, reviewable increments over broad rewrites.
- Preserve existing project conventions from [AGENTS.md](../../../AGENTS.md).
