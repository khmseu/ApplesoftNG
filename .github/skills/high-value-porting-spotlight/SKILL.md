# high-value-porting-spotlight

## Purpose

Automated identification of high-value assembly-to-C++ porting targets based on developer metrics and functional criticality.

## Procedure

### Target Identification

1. Scan [docs/function-cross-reference.md](../../../docs/function-cross-reference.md) for functions marked as `stub`.
2. Sort identified stubs by `Calls (Approx)` in descending order.
3. Select the top candidate that has not yet been started (check for matching `asm_port_*.cpp` files).

### Label Resolution

1. Lookup the selected candidate label in [SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.sym](../../../SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.sym).
2. Note the hexadecimal address for the label.
3. Open [SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst](../../../SourceMaterial/Apple-II-Source-slim/src/system/applesoft/applesoft.o65.lst) and navigate to that address.
4. Identify the logical end of the function (typically an `RTS`, `JMP`, or the start of the next major named label).
5. Verify if a clean `end_label` exists in the symbol table.
6. Assume labels of the form L_4hex are most likely local and not function delimiters.

### Handover

1. Once the `start_label` and `end_label` are determined, invoke the `asm-incremental-port` skill.
2. Provide the range and any discovered context about 16-bit pointers or fall-throughs.
3. Continue immediately into the `asm-incremental-port` workflow in the same response.

## Definition Of Done

- Top stub candidate from cross-reference is identified.
- Valid label range (start_label, end_label) is resolved in assembly and symbols.
- Task is handed over to `asm-incremental-port` and the next phase starts immediately.
