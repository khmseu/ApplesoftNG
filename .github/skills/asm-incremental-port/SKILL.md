# asm-incremental-port

## Purpose
Incrementally port Applesoft historical assembler ranges into C++ using a strict two-label conversion window.

## Trigger Phrases
- "convert labels X to Y"
- "port asm range"
- "incremental assembler to c++"
- "translate this listing window"

## Inputs
- `start_label`
- `end_label`

Optional:
- preferred destination subsystem (`core` or `platform`)

## Procedure
1. Resolve both labels in the listings under [SourceMaterial/Apple-II-Source-slim/src/system](../../../SourceMaterial/Apple-II-Source-slim/src/system).
2. Read the full bounded range, including inline comments and comments immediately preceding the range.
3. Summarize intent in 2-5 bullets before coding.
4. Implement one C++ function for the range.
5. Keep label-based naming as-is when legal; otherwise minimally normalize and document mapping.
6. Choose destination by behavior:
- language/runtime semantics -> [src/core](../../../src/core)
- device/console/monitor I/O semantics -> [src/platform](../../../src/platform)
7. Add missing dependency stubs near the new implementation with `TODO(asm-port)` markers.
8. Build and report the exact files changed.

## Definition Of Done
- Conversion is bounded by requested labels.
- Behavior and comments from the source range are reflected in code comments.
- New function compiles.
- Any unresolved dependencies are represented by dummy implementations.
- No runtime reads from SourceMaterial.

## Notes
- Prefer small, reviewable increments over broad rewrites.
- Preserve existing project conventions from [AGENTS.md](../../../AGENTS.md).
