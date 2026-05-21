---
description: "Find the next unclaimed ASM label after a start address and port the bounded range"
argument-hint: "Start address, e.g. $d000"
agent: "agent"
---

# Port the next unclaimed ASM range

Use the workspace symbol map at [docs/symbol-implementation-map.tsv](../../docs/symbol-implementation-map.tsv) to identify a bounded range, then hand that range to [port-asm-label-range.prompt.md](./port-asm-label-range.prompt.md).

Required input:

- `start_address` (inclusive search start, e.g. `$d000`)

## Execution contract

### Range Selection

1. Inspect [docs/symbol-implementation-map.tsv](../../docs/symbol-implementation-map.tsv) starting at `start_address`.
2. Find the next label `A` whose implemented-by column is `-`.
3. Find the next claimed label `B` after `A` whose implemented-by column is not `-`.
4. If either label cannot be found, return an error naming the missing label and stop.

### Porting

1. Invoke the workflow in [port-asm-label-range.prompt.md](./port-asm-label-range.prompt.md) with `A` as `start_label` and `B` as `end_label`.
2. Follow the response format and finalization requirements from that prompt exactly.

## Response format

- Selected labels `A` and `B`.
- Source file located.
- Behavior summary.
- Files changed.
- Any stubs created.
- Pointer candidates identified and how each was lifted/unified.
- Whether `AS_Labels` claims were added/updated.
- If split, which function maps to which claimed sub-range.
- Build result.
