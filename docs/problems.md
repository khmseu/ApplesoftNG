# Unification Problems

## Current Snapshot

The original stub-shadowing list is mostly retired. The handlers below now live in their canonical modules:

| Function           | Canonical location                                                              | Notes                                                          |
| ------------------ | ------------------------------------------------------------------------------- | -------------------------------------------------------------- |
| CHRGET             | [src/core/asm_port_chrget.cpp](../src/core/asm_port_chrget.cpp)                 | Real implementation.                                           |
| CHRGOT             | [src/core/asm_port_chrget.cpp](../src/core/asm_port_chrget.cpp)                 | Real implementation.                                           |
| FRMEVL             | [src/core/asm_port_core.cpp](../src/core/asm_port_core.cpp)                     | Core evaluator entry point; parser and string helpers call it. |
| HANDLERR           | [src/core/asm_port_core.cpp](../src/core/asm_port_core.cpp)                     | Real handler.                                                  |
| STROUT             | [src/core/asm_port_print.cpp](../src/core/asm_port_print.cpp)                   | Real string/output routine.                                    |
| SYNCHR             | [src/core/asm_port_parser.cpp](../src/core/asm_port_parser.cpp)                 | Real syntax-check helper.                                      |
| SYNERR             | [src/core/asm_port_error_handling.cpp](../src/core/asm_port_error_handling.cpp) | Real syntax-error handler.                                     |
| SETDA              | [src/core/asm_port_clear.cpp](../src/core/asm_port_clear.cpp)                   | Real data-pointer setter.                                      |
| PRINT_ERROR_LINNUM | [src/core/asm_port_error_handling.cpp](../src/core/asm_port_error_handling.cpp) | Intentional overload pair; one bare form and one prefix form.  |

## Still Intentional

| Function    | Canonical location                                                                                                                                                                                                | Notes                                                                       |
| ----------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------- |
| MEMERR      | [src/core/asm_port_chkmem.cpp](../src/core/asm_port_chkmem.cpp), [src/core/asm_port_error_handling.cpp](../src/core/asm_port_error_handling.cpp), [src/core/asm_port_reason.cpp](../src/core/asm_port_reason.cpp) | Intentional state-machine overloads with different signatures.              |
| AS_ERROR_fn | [src/core/asm_port_unfnc.cpp](../src/core/asm_port_unfnc.cpp)                                                                                                                                                     | Dispatch-table alias for the SCRN( token slot; not a duplicate of AS_ERROR. |

## Notes

- The old stub-only list is no longer useful as a porting target.
- If a future refactor reintroduces a shadowed helper, add it here only if the code path is still externally visible and actually ambiguous.
