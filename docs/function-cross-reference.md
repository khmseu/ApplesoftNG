# Function Cross Reference

Generated from C++ source definitions in `src/` by scanning top-level (non-static,
non-member) function definitions. **Excludes**: `static` function bodies in
`asm_port_unfnc.cpp`, class member definitions (`Interpreter::`, `Memory::`,
`ApplesoftVariables::`). See the supplementary section below for those entries.

Status key — **real**: non-trivial body present; **stub**: body is empty, trivially
returning, or contains `TODO(asm-port)`.

| Function                          | Implemented In                            | Line | Status |
| --------------------------------- | ----------------------------------------- | ---: | ------ |
| ADDON                             | src/core/asm_port_error.cpp               | 1165 | real   |
| ANDOP                             | src/core/asm_port_error.cpp               | 2027 | stub   |
| ARRAY                             | src/core/asm_port_error.cpp               | 2407 | stub   |
| ASC                               | src/core/asm_port_strlt2.cpp              |  579 | real   |
| AYINT                             | src/core/asm_port_error.cpp               | 2372 | real   |
| AdvanceTextPointerToNextLine      | src/core/asm_port_error.cpp               | 1203 | real   |
| AdvanceToNextLine                 | src/core/asm_port_error.cpp               |  940 | real   |
| ApplyFacSign                      | src/core/asm_port_error.cpp               | 1064 | stub   |
| BADNAM                            | src/core/asm_port_error.cpp               | 2184 | real   |
| BASIC                             | src/core/asm_port_error.cpp               | 2202 | real   |
| BASIC2                            | src/core/asm_port_error.cpp               | 2210 | real   |
| BLTU                              | src/core/asm_port_bltu.cpp                |   21 | stub   |
| BLTU2                             | src/core/asm_port_bltu.cpp                |   36 | real   |
| CAT                               | src/core/asm_port_strlt2.cpp              |  418 | real   |
| CHECK_BUMP                        | src/core/asm_port_strlt2.cpp              |  316 | real   |
| CHECK_EXIT                        | src/core/asm_port_strlt2.cpp              |  324 | real   |
| CHECK_SIMPLE_VARIABLE             | src/core/asm_port_strlt2.cpp              |  262 | real   |
| CHECK_VARIABLE                    | src/core/asm_port_strlt2.cpp              |  282 | real   |
| CHKCOM                            | src/core/asm_port_error.cpp               | 1104 | real   |
| CHKMEM                            | src/core/asm_port_chkmem.cpp              |   16 | real   |
| CHKNUM                            | src/core/asm_port_error.cpp               | 1080 | real   |
| CHKSTR                            | src/core/asm_port_error.cpp               | 1092 | real   |
| CHKVAL                            | src/core/asm_port_error.cpp               | 1953 | stub   |
| CHRGET                            | src/core/asm_port_error.cpp               |   87 | real   |
| CHRGET_INPUT                      | src/core/asm_port_input.cpp               |   33 | stub   |
| CHRGOT                            | src/core/asm_port_error.cpp               |   77 | real   |
| CHRSTR                            | src/core/asm_port_strlt2.cpp              |  599 | real   |
| CLEAR                             | src/core/asm_port_error.cpp               |  855 | real   |
| CLEAR                             | src/core/asm_port_token_address_table.cpp |    9 | stub   |
| CLEARC                            | src/core/asm_port_error.cpp               |  863 | real   |
| CLEARC_impl                       | src/core/asm_port_error.cpp               |  328 | stub   |
| CLEAR_impl                        | src/core/asm_port_error.cpp               |  319 | real   |
| CLREOL                            | src/platform/asm_port_getln.cpp           |   24 | stub   |
| CMPDONE                           | src/core/asm_port_error.cpp               | 2109 | real   |
| COLD_START                        | src/core/asm_port_error.cpp               |  108 | real   |
| COLON\_                           | src/core/asm_port_error.cpp               | 1257 | real   |
| CONINT                            | src/core/asm_port_error.cpp               |  435 | real   |
| COMBYTE                           | src/core/asm_port_error.cpp               | 2448 | real   |
| CONT                              | src/core/asm_port_error.cpp               | 1353 | real   |
| CONT                              | src/core/asm_port_token_address_table.cpp |   16 | stub   |
| CONTROL_C_TYPED                   | src/core/asm_port_error.cpp               | 1281 | real   |
| CONTROL_C_TYPED                   | src/core/asm_port_input.cpp               |   18 | stub   |
| COUT                              | src/platform/asm_port_getln.cpp           |   18 | stub   |
| CRDO                              | src/core/asm_port_print.cpp               |  234 | real   |
| CROUT                             | src/platform/asm_port_getln.cpp           |   33 | stub   |
| C_ZERO                            | src/core/asm_port_error.cpp               | 2285 | real   |
| ClearErrFlag                      | src/core/asm_port_error.cpp               |   35 | real   |
| CompareArgAndFacStrings           | src/core/asm_port_error.cpp               |  462 | stub   |
| CurrentStatementChar              | src/core/asm_port_error.cpp               | 1217 | real   |
| DATA                              | src/core/asm_port_error.cpp               | 1711 | real   |
| DATA                              | src/core/asm_port_token_address_table.cpp |   23 | stub   |
| DATAN                             | src/core/asm_port_error.cpp               | 1176 | stub   |
| DEF                               | src/core/asm_port_error.cpp               | 2678 | stub   |
| DIM                               | src/core/asm_port_error.cpp               | 2148 | real   |
| DIM                               | src/core/asm_port_token_address_table.cpp |   27 | stub   |
| DeleteExistingLine                | src/core/asm_port_error.cpp               |  822 | real   |
| ENDX                              | src/core/asm_port_error.cpp               | 1307 | real   |
| ENDX                              | src/core/asm_port_token_address_table.cpp |   14 | stub   |
| ENDX_impl                         | src/core/asm_port_error.cpp               | 1311 | real   |
| ERLIN                             | src/core/asm_port_input.cpp               |  203 | real   |
| ERRDIR                            | src/core/asm_port_error.cpp               | 2641 | real   |
| ERROR                             | src/core/asm_port_error.cpp               |  871 | real   |
| ERROR_MESSAGES                    | src/core/asm_port_error_messages.cpp      |   14 | stub   |
| EXECUTE_STATEMENT                 | src/core/asm_port_error.cpp               | 1222 | real   |
| EXECUTE_STATEMENT_1               | src/core/asm_port_error.cpp               | 1235 | real   |
| FADD                              | src/core/asm_port_error.cpp               |  398 | stub   |
| FAE_1                             | src/core/asm_port_error.cpp               |  276 | real   |
| FALSE                             | src/core/asm_port_error.cpp               | 2044 | real   |
| FCOMP                             | src/core/asm_port_error.cpp               |  421 | stub   |
| FCOMP2                            | src/core/asm_port_error.cpp               |  401 | stub   |
| FINDATA                           | src/core/asm_port_input.cpp               |   60 | stub   |
| FIND_ARRAY_ELEMENT                | src/core/asm_port_error.cpp               |  275 | real   |
| FIND_HIGHEST_STRING               | src/core/asm_port_strlt2.cpp              |  209 | real   |
| FIX_LINKS                         | src/core/asm_port_error.cpp               |  956 | stub   |
| FL1                               | src/core/asm_port_error.cpp               | 1692 | stub   |
| FLOAT                             | src/core/asm_port_error.cpp               |  432 | stub   |
| FNCDATA                           | src/core/asm_port_error.cpp               | 2714 | stub   |
| FNC\_                             | src/core/asm_port_error.cpp               | 2690 | stub   |
| FNDLIN                            | src/core/asm_port_error.cpp               |  643 | real   |
| FOR                               | src/core/asm_port_error.cpp               |  350 | stub   |
| FOR                               | src/core/asm_port_token_address_table.cpp |   38 | stub   |
| FRE                               | src/core/asm_port_error.cpp               | 2592 | real   |
| FREFAC                            | src/core/asm_port_print.cpp               |   51 | real   |
| FREFAC                            | src/core/asm_port_strlt2.cpp              |  493 | real   |
| FRESTR                            | src/core/asm_port_strlt2.cpp              |  485 | real   |
| FRETMP                            | src/core/asm_port_strlt2.cpp              |  500 | real   |
| FRMEVL                            | src/core/asm_port_error.cpp               | 2099 | real   |
| FRMNUM                            | src/core/asm_port_error.cpp               | 1112 | real   |
| FRM_STACK_2                       | src/core/asm_port_error.cpp               | 1127 | stub   |
| FRM_STACK_3                       | src/core/asm_port_error.cpp               | 1131 | stub   |
| FUNCT                             | src/core/asm_port_error.cpp               | 2702 | stub   |
| GARBAG                            | src/core/asm_port_reason.cpp              |   14 | stub   |
| GARBAG                            | src/core/asm_port_strlt2.cpp              |  200 | real   |
| GENERIC_CHRGET                    | src/core/asm_port_error.cpp               |   59 | real   |
| GENERIC_END                       | src/core/asm_port_error.cpp               |   99 | real   |
| GET                               | src/core/asm_port_input.cpp               |  258 | real   |
| GET                               | src/core/asm_port_token_address_table.cpp |   24 | stub   |
| GETARY                            | src/core/asm_port_error.cpp               | 2315 | real   |
| GETARY2                           | src/core/asm_port_error.cpp               | 2323 | real   |
| GETADR                            | src/core/asm_port_error.cpp               | 2457 | real   |
| GETBYT                            | src/core/asm_port_error.cpp               | 1937 | real   |
| GETCHR                            | src/core/asm_port_error.cpp               |  610 | real   |
| GETLN                             | src/platform/asm_port_getln.cpp           |   37 | stub   |
| GETLNZ                            | src/platform/asm_port_getln.cpp           |   27 | stub   |
| GETSPA                            | src/core/asm_port_strlt2.cpp              |  170 | real   |
| GETSTR                            | src/core/asm_port_strlt2.cpp              |  571 | real   |
| GTNUM                             | src/core/asm_port_error.cpp               | 2440 | real   |
| GIVAYF                            | src/core/asm_port_error.cpp               | 2613 | real   |
| GME                               | src/core/asm_port_error.cpp               |  278 | real   |
| GOEND                             | src/core/asm_port_error.cpp               | 1184 | real   |
| GOSUB                             | src/core/asm_port_error.cpp               | 1500 | stub   |
| GOSUB                             | src/core/asm_port_token_address_table.cpp |   20 | stub   |
| GOTO                              | src/core/asm_port_error.cpp               | 1544 | stub   |
| GOTO                              | src/core/asm_port_token_address_table.cpp |   21 | stub   |
| GO_TO_LINE                        | src/core/asm_port_error.cpp               | 1534 | real   |
| GSE                               | src/core/asm_port_error.cpp               | 2505 | real   |
| GTBYTC                            | src/core/asm_port_error.cpp               |      | real   |
| GTFORPNT                          | src/core/asm_port_gtforpnt.cpp            |   25 | stub   |
| GetTextTablePointer               | src/core/asm_port_error.cpp               |  931 | real   |
| HANDLERR                          | src/core/asm_port_error.cpp               | 2538 | real   |
| HANDLERR                          | src/core/asm_port_input.cpp               |   19 | stub   |
| HandleNumberedLine                | src/core/asm_port_error.cpp               |  835 | real   |
| IF                                | src/core/asm_port_error.cpp               | 1720 | stub   |
| IF                                | src/core/asm_port_token_address_table.cpp |   29 | real   |
| IF_TRUE                           | src/core/asm_port_error.cpp               | 1752 | real   |
| INCHR                             | src/core/asm_port_inlin2.cpp              |   68 | real   |
| INLIN                             | src/core/asm_port_inlin.cpp               |    7 | real   |
| INLIN2                            | src/core/asm_port_inlin2.cpp              |   43 | real   |
| INPERR                            | src/core/asm_port_input.cpp               |  232 | real   |
| INPRT                             | src/core/asm_port_error.cpp               |  983 | real   |
| INPUT                             | src/core/asm_port_input.cpp               |   75 | real   |
| INPUT                             | src/core/asm_port_token_address_table.cpp |   25 | stub   |
| INPUTERR                          | src/core/asm_port_input.cpp               |  240 | real   |
| INPUT_FLAG_ZERO                   | src/core/asm_port_input.cpp               |  116 | real   |
| IQERR                             | src/core/asm_port_error.cpp               | 2430 | real   |
| ISCNTC                            | src/core/asm_port_error.cpp               | 1014 | real   |
| ISLETC                            | src/core/asm_port_error.cpp               | 2267 | real   |
| InsertNewLine                     | src/core/asm_port_error.cpp               |  826 | real   |
| IsDirectMode                      | src/core/asm_port_error.cpp               | 2547 | real   |
| IsEndOfLineAtTextPointer          | src/core/asm_port_error.cpp               | 1188 | real   |
| IsEndOfProgram                    | src/core/asm_port_error.cpp               |  936 | real   |
| IsEndOfProgramAtTextPointer       | src/core/asm_port_error.cpp               | 1193 | real   |
| IsLineNumberGreater               | src/core/asm_port_error.cpp               |  617 | real   |
| IsOnErr                           | src/core/asm_port_error.cpp               | 2542 | real   |
| IsRunningMode                     | src/core/asm_port_error.cpp               | 1207 | real   |
| IsStatementEndOfParsedInput       | src/core/asm_port_error.cpp               |  157 | real   |
| IsTraceEnabled                    | src/core/asm_port_error.cpp               | 1212 | real   |
| JER                               | src/core/asm_port_error.cpp               | 2439 | real   |
| JERR                              | src/core/asm_port_strlt2.cpp              |  140 | real   |
| LEFTSTR                           | src/core/asm_port_strlt2.cpp              |  668 | real   |
| LEN                               | src/core/asm_port_strlt2.cpp              |  563 | real   |
| LET                               | src/core/asm_port_error.cpp               | 1818 | stub   |
| LET                               | src/core/asm_port_token_address_table.cpp |   28 | stub   |
| LET2                              | src/core/asm_port_error.cpp               | 1847 | stub   |
| LINGET                            | src/core/asm_port_error.cpp               |   51 | stub   |
| LINPRT                            | src/core/asm_port_error.cpp               |  608 | real   |
| LIST                              | src/core/asm_port_error.cpp               |  788 | real   |
| LIST                              | src/core/asm_port_token_address_table.cpp |   11 | stub   |
| LOAD                              | src/core/asm_port_error.cpp               | 1398 | stub   |
| LOAD                              | src/core/asm_port_token_address_table.cpp |   17 | stub   |
| LOAD_FAC_FROM_YA                  | src/core/asm_port_error.cpp               | 1072 | stub   |
| MAKE_NEW_ARRAY                    | src/core/asm_port_error.cpp               | 2464 | real   |
| MAKE_NEW_VARIABLE                 | src/core/asm_port_error.cpp               | 2294 | stub   |
| MAKINT                            | src/core/asm_port_error.cpp               | 2345 | real   |
| MATHTBL                           | src/core/asm_port_mathtbl.cpp             |   35 | real   |
| MEMERR                            | src/core/asm_port_chkmem.cpp              |    9 | real   |
| MEMERR                            | src/core/asm_port_error.cpp               |  212 | stub   |
| MEMERR                            | src/core/asm_port_memerr.cpp              |    8 | real   |
| MEMERR                            | src/core/asm_port_reason.cpp              |   19 | real   |
| MI1                               | src/core/asm_port_error.cpp               | 2391 | real   |
| MI2                               | src/core/asm_port_error.cpp               | 2399 | real   |
| MIDSTR                            | src/core/asm_port_strlt2.cpp              |  695 | real   |
| MKINT                             | src/core/asm_port_error.cpp               | 2355 | real   |
| MON_COUT                          | src/platform/asm_port_outdo.cpp           |   16 | real   |
| MON_GETLN                         | src/core/asm_port_inlin2.cpp              |   17 | real   |
| MON_PREAD                         | src/core/asm_port_error.cpp               |  438 | stub   |
| MON_RDKEY                         | src/core/asm_port_inlin2.cpp              |   39 | real   |
| MON_READ                          | src/core/asm_port_error.cpp               | 1803 | stub   |
| MON_SCRN                          | src/core/asm_port_error.cpp               |  416 | stub   |
| MON_WAIT                          | src/platform/asm_port_outdo.cpp           |   30 | stub   |
| MON_WRITE                         | src/core/asm_port_error.cpp               | 1799 | stub   |
| MOVE_HIGHEST_STRING_TO_TOP        | src/core/asm_port_strlt2.cpp              |  331 | real   |
| MOVINS                            | src/core/asm_port_strlt2.cpp              |  449 | real   |
| MOVSTR                            | src/core/asm_port_strlt2.cpp              |  460 | real   |
| MOVSTR_1                          | src/core/asm_port_strlt2.cpp              |  469 | real   |
| MULTIPLY_SUBSCRIPT                | src/core/asm_port_error.cpp               | 2523 | real   |
| MULTIPLY_SUBS_1                   | src/core/asm_port_error.cpp               | 2539 | real   |
| MarkDirectMode                    | src/core/asm_port_error.cpp               |   39 | real   |
| MatchToken                        | src/core/asm_port_error.cpp               |  681 | real   |
| NAME_NOT_FOUND                    | src/core/asm_port_error.cpp               | 2276 | stub   |
| NAMOK                             | src/core/asm_port_error.cpp               | 2192 | real   |
| NEG32768                          | src/core/asm_port_error.cpp               | 2334 | real   |
| NEGATE                            | src/core/asm_port_print.cpp               |  253 | real   |
| NEW                               | src/core/asm_port_error.cpp               |  847 | real   |
| NEW                               | src/core/asm_port_token_address_table.cpp |   10 | stub   |
| NEWSTT                            | src/core/asm_port_error.cpp               |  551 | real   |
| NEW_impl                          | src/core/asm_port_error.cpp               |  280 | real   |
| NEXT                              | src/core/asm_port_error.cpp               |  468 | stub   |
| NEXT                              | src/core/asm_port_token_address_table.cpp |   15 | stub   |
| NEXT_shouldTerminateLoop          | src/core/asm_port_error.cpp               |  404 | stub   |
| NOTCR                             | src/platform/asm_port_getln.cpp           |   30 | stub   |
| NUMCMP                            | src/core/asm_port_error.cpp               | 2099 | real   |
| NXDIM                             | src/core/asm_port_error.cpp               | 2139 | real   |
| NXIN                              | src/core/asm_port_nxin.cpp                |   13 | real   |
| ONERR                             | src/core/asm_port_error.cpp               | 3510 | real   |
| ONERR                             | src/core/asm_port_token_address_table.cpp |   96 | real   |
| ONGOTO                            | src/core/asm_port_error.cpp               | 1765 | stub   |
| ONGOTO                            | src/core/asm_port_token_address_table.cpp |   31 | stub   |
| OR                                | src/core/asm_port_error.cpp               | 2010 | stub   |
| OUTDO                             | src/platform/asm_port_outdo.cpp           |   39 | real   |
| OUTQUES                           | src/platform/asm_port_outdo.cpp           |   75 | real   |
| OUTSP                             | src/platform/asm_port_outdo.cpp           |   67 | real   |
| PARCHK                            | src/core/asm_port_error.cpp               | 2726 | stub   |
| PARSE_INPUT_LINE                  | src/core/asm_port_error.cpp               |  722 | stub   |
| PDL                               | src/core/asm_port_error.cpp               | 2130 | real   |
| PDL                               | src/core/asm_port_unfnc.cpp               |   13 | stub   |
| PEEK                              | src/core/asm_port_error.cpp               | 2474 | real   |
| PLOTFNS                           | src/core/asm_port_error.cpp               |  413 | stub   |
| POINT                             | src/core/asm_port_strtxt.cpp              |   24 | stub   |
| POKE                              | src/core/asm_port_error.cpp               | 2487 | real   |
| POP                               | src/core/asm_port_error.cpp               | 1578 | real   |
| POP                               | src/core/asm_port_token_address_table.cpp |   22 | stub   |
| POS                               | src/core/asm_port_error.cpp               | 2633 | real   |
| PRINT                             | src/core/asm_port_print.cpp               |  202 | real   |
| PRINT2                            | src/core/asm_port_print.cpp               |  219 | real   |
| PRINT_ERROR_LINNUM                | src/core/asm_port_error.cpp               | 1341 | real   |
| PROCESS_INPUT_LIST                | src/core/asm_port_input.cpp               |  123 | stub   |
| PROGIO                            | src/core/asm_port_error.cpp               | 1449 | stub   |
| PR_COMMA                          | src/core/asm_port_print.cpp               |  268 | real   |
| PR_STRING                         | src/core/asm_port_print.cpp               |  181 | real   |
| PR_TAB_OR_SPC                     | src/core/asm_port_print.cpp               |  296 | real   |
| PTRGET                            | src/core/asm_port_error.cpp               | 1942 | real   |
| PTRGET2                           | src/core/asm_port_error.cpp               | 2161 | real   |
| PTRGET3                           | src/core/asm_port_error.cpp               | 2169 | real   |
| PTRGET4                           | src/core/asm_port_error.cpp               | 2218 | stub   |
| PULL3                             | src/core/asm_port_error.cpp               | 1633 | real   |
| PUTEMP                            | src/core/asm_port_strlt2.cpp              |  147 | real   |
| PUTNEW                            | src/core/asm_port_strlt2.cpp              |  124 | real   |
| PUTSTR                            | src/core/asm_port_error.cpp               | 1871 | stub   |
| PeekTopControlTokenAfterGTFORPNT  | src/core/asm_port_error.cpp               | 1706 | real   |
| PopByteFromStack                  | src/core/asm_port_error.cpp               | 1696 | real   |
| PopReturnAddress                  | src/core/asm_port_error.cpp               | 1031 | real   |
| PopWordFromStack                  | src/core/asm_port_error.cpp               | 1045 | real   |
| PrintListLine                     | src/core/asm_port_error.cpp               |  628 | stub   |
| PushByteToStack                   | src/core/asm_port_error.cpp               | 1035 | real   |
| PushCurrentLineNumber             | src/core/asm_port_error.cpp               | 1056 | real   |
| PushForPntFrame                   | src/core/asm_port_error.cpp               | 1811 | real   |
| PushTextPointerAddress            | src/core/asm_port_error.cpp               | 1052 | real   |
| PushTokenTo                       | src/core/asm_port_error.cpp               | 1060 | real   |
| PushWordToStack                   | src/core/asm_port_error.cpp               | 1039 | real   |
| QINT                              | src/core/asm_port_error.cpp               |  443 | stub   |
| QT_ERROR                          | src/core/asm_port_qt_error.cpp            |   16 | real   |
| RDCHAR                            | src/platform/asm_port_getln.cpp           |   21 | stub   |
| RDKEY                             | src/core/asm_port_inlin2.cpp              |   34 | stub   |
| READ                              | src/core/asm_port_input.cpp               |  105 | real   |
| READ                              | src/core/asm_port_token_address_table.cpp |   26 | stub   |
| READERR                           | src/core/asm_port_input.cpp               |  211 | stub   |
| REASON                            | src/core/asm_port_reason.cpp              |   26 | real   |
| RELOPS                            | src/core/asm_port_error.cpp               | 2060 | stub   |
| REM                               | src/core/asm_port_error.cpp               | 1743 | real   |
| REM                               | src/core/asm_port_token_address_table.cpp |   30 | real   |
| REMN                              | src/core/asm_port_error.cpp               | 1643 | real   |
| RESPERR                           | src/core/asm_port_input.cpp               |  218 | stub   |
| RESTART                           | src/core/asm_port_error.cpp               |  904 | stub   |
| RESTORE                           | src/core/asm_port_error.cpp               | 1269 | real   |
| RESTORE                           | src/core/asm_port_token_address_table.cpp |   12 | stub   |
| RESUME                            | src/core/asm_port_error.cpp               | 3490 | real   |
| RESUME                            | src/core/asm_port_token_address_table.cpp |   92 | real   |
| RETURN                            | src/core/asm_port_error.cpp               | 1607 | real   |
| RIGHTSTR                          | src/core/asm_port_strlt2.cpp              |  681 | real   |
| ROUND_FAC                         | src/core/asm_port_error.cpp               | 2368 | stub   |
| RTS_5                             | src/core/asm_port_error.cpp               | 1629 | stub   |
| RUN                               | src/core/asm_port_error.cpp               | 1469 | stub   |
| RUN                               | src/core/asm_port_token_address_table.cpp |   19 | stub   |
| ReadLineNumberFromTextPointer     | src/core/asm_port_error.cpp               | 1198 | stub   |
| ReadProgramByte                   | src/core/asm_port_error.cpp               |  597 | stub   |
| ReadProgramLineNumber             | src/core/asm_port_error.cpp               |  624 | real   |
| ReadStackPointer                  | src/core/asm_port_error.cpp               | 1026 | real   |
| ReturnWasFromPOPContext           | src/core/asm_port_error.cpp               | 1701 | real   |
| SAVE                              | src/core/asm_port_error.cpp               | 1378 | stub   |
| SAVE                              | src/core/asm_port_token_address_table.cpp |   18 | stub   |
| SCREEN                            | src/core/asm_port_error.cpp               | 1971 | stub   |
| SCRTCH                            | src/core/asm_port_error.cpp               |  859 | real   |
| SCRTCH_impl                       | src/core/asm_port_error.cpp               |  289 | real   |
| SETDA                             | src/core/asm_port_error.cpp               | 1276 | real   |
| SETDA                             | src/core/asm_port_input.cpp               |   66 | real   |
| SETFOR                            | src/core/asm_port_error.cpp               | 2521 | stub   |
| SETPTRS                           | src/core/asm_port_error.cpp               |  851 | real   |
| SETPTRS_impl                      | src/core/asm_port_error.cpp               |  314 | real   |
| SET_VARPNT_AND_YA                 | src/core/asm_port_error.cpp               | 2306 | real   |
| SIGN                              | src/core/asm_port_error.cpp               | 1123 | stub   |
| SNGFLT                            | src/core/asm_port_error.cpp               |  462 | real   |
| STEP                              | src/core/asm_port_error.cpp               |  536 | stub   |
| STKINI                            | src/core/asm_port_error.cpp               |  987 | real   |
| STOP                              | src/core/asm_port_error.cpp               | 1295 | real   |
| STOP                              | src/core/asm_port_token_address_table.cpp |   13 | stub   |
| STOP_impl                         | src/core/asm_port_error.cpp               | 1299 | real   |
| STORE_FACDB_YX_ROUNDED            | src/core/asm_port_error.cpp               | 2730 | stub   |
| STR                               | src/core/asm_port_strlit.cpp              |   28 | real   |
| STRCMP                            | src/core/asm_port_error.cpp               | 2080 | stub   |
| STRINI                            | src/core/asm_port_strlt2.cpp              |   97 | real   |
| STRLIT                            | src/core/asm_port_strlit.cpp              |   41 | real   |
| STRLIT_from_word                  | src/core/asm_port_strtxt.cpp              |   19 | stub   |
| STRLT2                            | src/core/asm_port_strlt2.cpp              |  194 | real   |
| STROUT                            | src/core/asm_port_error.cpp               |  976 | stub   |
| STROUT                            | src/core/asm_port_input.cpp               |   21 | stub   |
| STROUT                            | src/core/asm_port_print.cpp               |  103 | real   |
| STRPRT                            | src/core/asm_port_print.cpp               |   78 | real   |
| STRSPA                            | src/core/asm_port_strlt2.cpp              |  108 | real   |
| STRTXT                            | src/core/asm_port_strtxt.cpp              |   28 | stub   |
| STXTPT                            | src/core/asm_port_error.cpp               |  867 | real   |
| STXTPT_impl                       | src/core/asm_port_error.cpp               |  342 | stub   |
| SUBERR                            | src/core/asm_port_error.cpp               | 2421 | real   |
| SYNCHR                            | src/core/asm_port_error.cpp               | 1076 | stub   |
| SYNCHR                            | src/core/asm_port_input.cpp               |   36 | stub   |
| SYNERR                            | src/core/asm_port_error.cpp               | 1807 | real   |
| SYNERR                            | src/core/asm_port_input.cpp               |   20 | stub   |
| ScanAheadOffset                   | src/core/asm_port_error.cpp               | 1135 | stub   |
| SetBranchTargetToSTEP             | src/core/asm_port_error.cpp               | 1068 | stub   |
| SetTextPointer                    | src/core/asm_port_error.cpp               |   31 | real   |
| SetTextPointerToInputBufferMinus1 | src/core/asm_port_error.cpp               |  670 | stub   |
| TOKEN_ADDRESS_TABLE               | src/core/asm_port_token_address_table.cpp |   96 | stub   |
| TOKEN_NAME_TABLE                  | src/core/asm_port_token_name_table.cpp    |   15 | stub   |
| TRACE (statement handler)         | src/core/asm_port_token_address_table.cpp |   70 | real   |
| NOTRACE (statement handler)       | src/core/asm_port_token_address_table.cpp |   75 | real   |
| TRACE\_                           | src/core/asm_port_error.cpp               |  581 | real   |
| TRUE                              | src/core/asm_port_error.cpp               | 2052 | real   |
| UNARY                             | src/core/asm_port_error.cpp               | 1990 | stub   |
| UNDFNC                            | src/core/asm_port_error.cpp               | 2653 | real   |
| UNFNC                             | src/core/asm_port_unfnc.cpp               |   49 | stub   |
| USE_OLD_ARRAY                     | src/core/asm_port_error.cpp               | 2447 | real   |
| VARTIO                            | src/core/asm_port_error.cpp               | 1429 | stub   |
| WAIT                              | src/core/asm_port_error.cpp               | 2497 | real   |
| WriteForwardPointer               | src/core/asm_port_error.cpp               |  951 | real   |
| WriteProgramByte                  | src/core/asm_port_error.cpp               |  130 | stub   |
| copy_page_backward                | src/core/asm_port_bltu.cpp                |    7 | real   |
| has_room                          | src/core/asm_port_reason.cpp              |    8 | stub   |
| isDigit                           | src/core/asm_port_error.cpp               |   22 | stub   |
| main                              | src/main.cpp                              |    4 | real   |
| parseNumericInputAndStore         | src/core/asm_port_input.cpp               |   57 | stub   |
| parseStringInputAndStore          | src/core/asm_port_input.cpp               |   54 | stub   |
| popStackByte                      | src/core/asm_port_input.cpp               |   45 | stub   |
| print                             | src/platform/console.cpp                  |    4 | real   |
| readLine                          | src/platform/console.cpp                  |    8 | real   |
| readStackByteAt                   | src/core/asm_port_error.cpp               |  389 | stub   |
| readStackWordAt                   | src/core/asm_port_error.cpp               |  393 | stub   |
| read_CHARAC                       | src/core/asm_port_strlt2.cpp              |   10 | real   |
| read_ENDCHR                       | src/core/asm_port_strlt2.cpp              |   14 | real   |
| read_INPUT_BUFFER                 | src/core/asm_port_error.cpp               |  662 | stub   |
| read_INPUT_BUFFER_minus_1         | src/core/asm_port_inlin2.cpp              |   25 | stub   |
| read_TXTPTR                       | src/core/asm_port_strtxt.cpp              |   11 | real   |
| read_carry_flag                   | src/core/asm_port_strtxt.cpp              |   15 | stub   |
| read_prompt_char                  | src/platform/asm_port_getln.cpp           |    9 | stub   |
| read_screen_char_via_28_y         | src/platform/asm_port_getln.cpp           |   15 | stub   |
| stack_at                          | src/core/asm_port_gtforpnt.cpp            |   13 | real   |
| stack_word_at                     | src/core/asm_port_gtforpnt.cpp            |   17 | real   |
| write_CHARAC                      | src/core/asm_port_strlit.cpp              |   11 | real   |
| write_ENDCHR                      | src/core/asm_port_strlit.cpp              |   15 | real   |
| write_FAC                         | src/core/asm_port_strlt2.cpp              |   22 | real   |
| write_FAC_pointer                 | src/core/asm_port_strlt2.cpp              |   26 | stub   |
| write_INPUT_BUFFER                | src/core/asm_port_inlin2.cpp              |   21 | real   |
| write_INPUT_BUFFER_minus_1        | src/core/asm_port_inlin2.cpp              |   29 | stub   |
| write_INPUT_BUFFER_minus_5        | src/core/asm_port_error.cpp               |  666 | stub   |
| write_MON_PROMPT                  | src/core/asm_port_inlin2.cpp              |   12 | real   |
| write_STRNG1                      | src/core/asm_port_strlt2.cpp              |   18 | real   |
| write_STRNG2                      | src/core/asm_port_strlt2.cpp              |   30 | stub   |
| write_input_buffer                | src/platform/asm_port_getln.cpp           |   12 | stub   |

---

## Supplementary: Entries Not Captured by Scanner

The scanner only matches top-level non-static non-member function definitions.
The following are excluded and tracked manually.

### Class member functions

| Symbol                            | Class                | File                             | Status |
| --------------------------------- | -------------------- | -------------------------------- | ------ |
| `Interpreter()` constructor       | `Interpreter`        | src/core/interpreter.cpp         | real   |
| `Memory()` constructor            | `Memory`             | src/core/memory.cpp:4            | real   |
| `execute()` (two overloads)       | `Interpreter`        | src/core/interpreter.cpp:11,16   | real   |
| `readByte()` / `writeByte()` etc. | `ApplesoftVariables` | src/core/applesoft_variables.cpp | real   |
| `~Interpreter()` destructor       | `Interpreter`        | src/core/interpreter.cpp         | stub   |
| `~Memory()` destructor            | `Memory`             | src/core/memory.cpp:8            | real   |

### Static dispatch stubs in `asm_port_unfnc.cpp`

All 24 function-token handlers are `static` stubs pending port. Token range `$D2`–`$EA`.

| Token | C++ name   | Notes                       |
| ----- | ---------- | --------------------------- |
| $D2   | `SGN`      | SGN                         |
| $D3   | `INT_fn`   | INT (keyword-escaped)       |
| $D4   | `ABS`      |                             |
| $D5   | `USR`      | user JMP via zero-page      |
| $D6   | `FRE`      |                             |
| $D7   | `ERROR`    | SCRN( token dispatches here |
| $D8   | `PDL_fn`   | delegates to `PDL()`        |
| $D9   | `POS`      |                             |
| $DA   | `SQR`      |                             |
| $DB   | `RND`      |                             |
| $DC   | `LOG`      |                             |
| $DD   | `EXP`      |                             |
| $DE   | `COS`      |                             |
| $DF   | `SIN`      |                             |
| $E0   | `TAN`      |                             |
| $E1   | `ATN`      |                             |
| $E2   | `PEEK`     |                             |
| $E3   | `LEN`      |                             |
| $E4   | `STR`      | STR$                        |
| $E5   | `VAL`      |                             |
| $E6   | `ASC`      |                             |
| $E7   | `CHRSTR`   | CHR$                        |
| $E8   | `LEFTSTR`  | LEFT$                       |
| $E9   | `RIGHTSTR` | RIGHT$                      |
| $EA   | `MIDSTR`   | MID$                        |
