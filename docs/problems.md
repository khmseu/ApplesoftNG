# unification problems

## **STUB + REAL DUPLICATES** (18 cases)

Unification problem: Stubs shadow real implementations or have mismatched signatures.

| Function     | Stub Location                              | Real Location | Problem                                                                                                     |
| ------------ | ------------------------------------------ | ------------- | ----------------------------------------------------------------------------------------------------------- |
| **CHRGET**   | error.cpp:26, print.cpp:17 (both stubs!)   | -             | TWO stubs, NO real; both modules need the real implementation                                               |
| **CHRGOT**   | input.cpp:20, print.cpp:21 (both stubs!)   | -             | TWO stubs, NO real; architecture split between input & print                                                |
| **ERROR**    | unfnc.cpp:24                               | error.cpp:857 | Stub in unfnc module shadows error handler; zero-arg vs error-code-arg                                      |
| **FRMEVL**   | error.cpp:1937, print.cpp:25 (both stubs!) | -             | TWO stubs, NO real; both modules declare but don't implement                                                |
| **HANDLERR** | error.cpp:2542, print.cpp:38 (both stubs!) | -             | TWO stubs, NO real; both modules defer to nonexistent handler                                               |
| **STROUT**   | error.cpp:981                              | print.cpp:109 | **Signature mismatch**: string_view version in error vs (uint8, uint8) in print—**two different functions** |
| **SYNCHR**   | error.cpp:1069, input.cpp:32 (both stubs!) | -             | TWO stubs, NO real                                                                                          |
| **SYNERR**   | error.cpp:1808, print.cpp:34 (both stubs!) | -             | TWO stubs, NO real                                                                                          |

---

## **MULTIPLE REAL DEFINITIONS** (5 cases)

Unification problem: Legitimate overloads vs. accidental duplicates; distinct semantics.

| Function               | Real Locations                                                             | Problem                                                                                                                     |
| ---------------------- | -------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------- |
| **MEMERR**             | chkmem.cpp:9 (CHKMEMState&), memerr.cpp:8 (), reason.cpp:19 (REASONState&) | **Three different signatures**—**intentional state-machine overloads**, not duplicates; names poorly distinguish by context |
| **PRINT_ERROR_LINNUM** | error.cpp:1339 (string_view prefix), error.cpp:874 ()                      | **Overload intended**: one with prefix, one without; both real implementations in same file                                 |
| **ReadProgramByte**    | error.cpp:572 (uint16), error.cpp:936 (LineAddress, uint8)                 | **Legitimate overload**: address-space abstraction; 6502 address vs. structured LineAddress                                 |
| **SETDA**              | error.cpp:1268 (uint16 dataPointer), input.cpp:62 (uint16 data_ptr)        | **Likely duplicate**: same purpose & signature, split between modules; one should call the other                            |
| **execute**            | interpreter.cpp:11, interpreter.cpp:16                                     | **Multiple real in same file** (Google Test overload artifact, not application code)                                        |

---

## **OBSERVATIONS & SUMMARY**

### **Worst categories:**

1. **Stub-only modules with NO real** (11 symbols: `CHRGET`, `CHRGOT`, `FRMEVL`, `HANDLERR`, `INLIN` stubs, `SYNCHR`, `SYNERR`, plus `INPRT`/`LINPRT` misclassified)  
   → **Action**: Remove local stubs; include real implementation's header; direct call.

2. **State-machine overloads** (`MEMERR`, `ReadProgramByte`)  
   → **Action**: Keep; rename or document context-dependent use clearly.

3. **Same-module duplicates** (`PRINT_ERROR_LINNUM`)  
   → **Action**: Consolidate to single implementation + overload, or rename one to clarify intent.

4. **Cross-module duplicates** (`SETDA`, `PTRGET`)  
   → **Action**: Identify canonical version; have the other call it or consolidate.
