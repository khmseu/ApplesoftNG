---
description: This document serves as a technical specification for an AI agent tasked with converting 6502 Assembly code into modern 64-bit C++. The primary challenge is the "lifting" of 16-bit split-byte addresses into high-level C++ pointers while maintaining the semantics of the original 8-bit logic.
---

# Specification: 6502 to C++ Pointer Synthesis

## 1. Context & Objective

The goal is to translate 6502 assembly (8-bit registers, 16-bit address space) into C++ code that runs natively on 64-bit architectures.

Instead of a simple "emulator" approach (where memory is just a `uint8_t[65536]` array), we want a **Refactored Translation**. We aim to identify when two 8-bit memory locations or registers are being used together to represent a 16-bit pointer, and convert those into C++ pointer types (`T*`) or references.

## 2. The Core Challenge: Pointer Fragmentation

On the 6502, a 16-bit address (e.g., `IOPorts::ADDR_KEYBOARD`) is handled as two separate 8-bit values:

- **Low Byte (BAL):** The least significant 8 bits.
- **High Byte (BAH):** The most significant 8 bits.

**AI Task:** You must recognize patterns where the code loads, stores, or manipulates these bytes in tandem and "re-unify" them into a single C++ pointer variable.

<!-- # 16-Bit Pointer Problem -->

## Background

6502 code often represents a 16-bit address as two 8-bit bytes. In this codebase the canonical bridge is `ApplesoftDualPointer<T>`, which can hold either an emulated 16-bit address or a native `T*`.

## Current Model

- `ApplesoftDualPointer::emulated(address)` preserves 6502-style addressing.
- `ApplesoftDualPointer::native(ptr)` wraps an actual typed C++ pointer.
- `nativePointer()` resolves emulated addresses through `ApplesoftVariables::pointer(address).address()`.
- Emulated pointers are rejected for regions that do not have stable typed storage: zero page, the input-buffer sentinel/page, and I/O-mapped ranges in `IOPorts`.

## Practical Use

Use the dual-pointer type for ROM tables and other data where the port needs to switch between emulated addresses and native storage. Examples in the tree include token-name tables, token-address tables, ROM constants, error-message tables, and math tables.

Do not use it to hide genuine byte-level state. Use `ApplesoftVariables` or `IOPorts` directly when the code is intentionally modeling the original 8-bit memory layout.

## Why This Exists

The goal is to keep 6502 address semantics visible while still allowing native C++ code to work with typed objects and normal pointer arithmetic.
