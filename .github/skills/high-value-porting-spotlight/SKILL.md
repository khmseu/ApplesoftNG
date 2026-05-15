# high-value-porting-spotlight

## Purpose

Automated identification of high-value assembly-to-C++ porting targets based on developer metrics and functional criticality.

## Procedure

### Target Identification

1. Scan [docs/function-cross-reference.md](../../../docs/function-cross-reference.md) for functions marked as `stub`.
2. Sort identified stubs by `Calls (Approx)` in descending order.
3. Select the top candidate that has not yet been started (check for matching `asm_port_*.cpp` files).

### Label Resolution

1. Build an ordered candidate list from the sorted stubs and process one candidate at a time.
2. For the current candidate label, derive a stripped label by removing an `AS_` or `MON_` prefix when present.
3. Grep through [SourceMaterial/Apple-II-Source-slim/src/system/_/_.o65.sym](../../../SourceMaterial/Apple-II-Source-slim/src/system/) for lines matching `^<label>, 0x[0-9a-fA-F]{4}, 0, 0x0002$`.
4. If no exact matches are found for the full label, repeat the grep using the stripped label with the same pattern.
5. If more than one candidate remains, use the stripped-prefix area hint to filter candidates:
   - `AS_` implies Applesoft area candidates.
   - `MON_` implies Monitor area candidates.
6. If there is not exactly one candidate after filtering, log the reason and move to the next candidate.
7. If exactly one candidate remains, select that `.sym` file and symbol entry, record the hexadecimal address, and open the matching listing file:
   - Applesoft listing: [SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst](../../../SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst)
   - Monitor listing: the corresponding `.o65.lst` file under [SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/](../../../SourceMaterial/Apple-II-Source-slim/src/system/monitor/apple2plus/)
8. Determine a candidate `end_label` by locating a boundary instruction (`RTS` or `JMP`) or the next major named label listed in the selected symbol file, then verify that the `end_label` exists in symbols.
9. Treat labels of the form `L_4hex` as local and not function delimiters unless explicitly documented in the associated `.sym` file or `.lst` file.
10. If both `start_label` and `end_label` are valid, stop candidate processing and continue to handover.
11. If either label is invalid or ambiguous, log the reason and continue with the next candidate.
12. If all candidates are exhausted and no valid `start_label`/`end_label` pair is found, log the issue and terminate with a clear message.

### Handover

1. Once the `start_label` and `end_label` are determined, invoke the `asm-incremental-port` skill.
2. Provide the range and any discovered context about 16-bit pointers or fall-throughs.
3. Continue immediately into the `asm-incremental-port` workflow in the same response.

## Definition Of Done

- Top stub candidate from cross-reference is identified.
- Valid label range (start_label, end_label) is resolved in assembly and symbols.
- Task is handed over to `asm-incremental-port` and the next phase starts immediately.
