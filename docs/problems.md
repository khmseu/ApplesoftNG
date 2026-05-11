# Unification Problems

## **STUB + REAL DUPLICATES**

Unification problem: stubs shadow real implementations, or a function has no
real implementation at all despite appearing in multiple modules.

| Function          | Stub Location(s)             | Real Location | Problem                                                                                                                     |
| ----------------- | ---------------------------- | ------------- | --------------------------------------------------------------------------------------------------------------------------- |
| **CHRGET**        | error.cpp:26                 | —             | Stub only; no real implementation anywhere. Must port the zero-page self-modifying CHRGET logic.                            |
| **CHRGOT**        | input.cpp:24                 | error.cpp:606 | Real exists in error.cpp. Stub in input.cpp should be removed; callers in input.cpp should include the real header.         |
| **FRMEVL**        | error.cpp:1933               | —             | Stub only; no real. Expression evaluator not yet ported.                                                                    |
| **HANDLERR**      | error.cpp:2538, input.cpp:19 | —             | Two stubs, no real. ON ERR handler not yet ported.                                                                          |
| **STROUT**        | error.cpp:976, input.cpp:21  | print.cpp:103 | Two stubs shadow the real in print.cpp. Both stub sites should call the print.cpp implementation directly.                  |
| **SYNCHR**        | error.cpp:1076, input.cpp:36 | —             | Two stubs, no real. Token-syntax check not yet ported.                                                                      |
| **SYNERR**        | error.cpp:1807, input.cpp:20 | —             | Two stubs, no real. Syntax-error handler not yet ported.                                                                    |
| **ERROR** (unfnc) | unfnc.cpp static `ERROR()`   | error.cpp:871 | Static stub in unfnc dispatch table. The SCRN( token slot ($D7) dispatches to the local static, not the real ERROR handler. |

---

## **MULTIPLE REAL DEFINITIONS**

Unification problem: legitimate overloads vs. accidental cross-module duplicates.

| Function               | Real Locations                                                                     | Problem                                                                                                                      |
| ---------------------- | ---------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------- |
| **MEMERR**             | chkmem.cpp:9 (`CHKMEMState&`), memerr.cpp:8 (`()`), reason.cpp:19 (`REASONState&`) | **Three different signatures — intentional state-machine overloads**, not duplicates; names poorly distinguish context.      |
| **PRINT_ERROR_LINNUM** | error.cpp:888 (`()`), error.cpp:1341 (`string_view prefix`)                        | **Overload intended**: one bare, one with prefix. Both real, same file. No action needed.                                    |
| **SETDA**              | error.cpp:1276 (`uint16`), input.cpp:66 (`uint16`)                                 | **Cross-module duplicate**: same purpose and equivalent signature. One should call the other or they should be consolidated. |
| **REM**                | error.cpp:1743 (real handler), token_address_table.cpp:30 (table entry)            | **Not a conflict**: the token_address_table.cpp entry is an address-table reference, not a competing implementation.         |

---

## **OBSERVATIONS & SUMMARY**

### Changes since previous version

- **CHRGOT** partially resolved: `error.cpp:606` is now a real implementation.
  The stub in `input.cpp:24` remains and should be removed.
- **HANDLERR** and **SYNERR** stubs moved from `print.cpp` to `input.cpp`
  (no functional change; both remain unresolved).
- **STROUT** now has two stubs (`error.cpp:976` + `input.cpp:21`) vs. one
  previously. The `input.cpp` stub is new and equally needs to forward to
  `print.cpp`'s real.
- **FRMEVL** `print.cpp` stub removed; only `error.cpp:1933` stub remains.
- **CHRGET** `print.cpp` stub removed; only `error.cpp:26` stub remains.
- `GetTextTableAddress` renamed to `GetTextTablePointer` (error.cpp:931).

### Worst categories

1. **Stub-only with no real** (`CHRGET`, `FRMEVL`, `HANDLERR`, `SYNCHR`, `SYNERR`)
   → Port the assembly logic; remove duplicate stubs once a real is available.

2. **Stubs shadowing an existing real** (`CHRGOT` in input.cpp, `STROUT` in
   error.cpp and input.cpp)
   → Delete stubs; add include of the real implementation's header.

3. **Cross-module duplicates** (`SETDA`)
   → Identify canonical; have the other call it or consolidate into one module.

4. **State-machine overloads** (`MEMERR`)
   → Keep; document context-dependent use clearly or rename per context.
