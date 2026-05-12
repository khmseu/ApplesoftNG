#!/usr/bin/env python3
"""
Regenerate docs/function-cross-reference.md from C++ source files.

Scans all src/**/*.cpp files for uppercase Applesoft/monitor function
definitions, classifies each as 'real' or 'stub', and writes a sorted
Markdown table to docs/function-cross-reference.md.

Inclusion rules
---------------
- Function name must match [A-Z][A-Z0-9_]* (all-uppercase identifiers).
- In src/core/asm_port_token_address_table.cpp: both static and non-static
  definitions are included (handlers live there as `static void FOO_Handler()`).
- In all other files: non-static function definitions are included, EXCEPT:
    * Static functions (file-local helpers) are always excluded in non-table files.
    * Functions inside a `namespace { }` (anonymous namespace) are excluded
      unless their name begins with `MON_` — those represent real monitor-layer
      functions that are tracked regardless of namespace placement.

Stub classification
-------------------
A function body is classified as 'stub' if ANY of the following hold:
  1. The body (between { and matching }) is empty after stripping whitespace
     and single-line comments.
  2. The body contains the marker `TODO(asm-port)`.
  3. The body contains only stub-return patterns: `return 0;`, `return false;`,
     `return nullptr;`, `return {};`, or `(void)expr;` statements.
Otherwise it is classified 'real'.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC_ROOT = REPO_ROOT / "src"
OUTPUT = REPO_ROOT / "docs" / "function-cross-reference.md"

# Relative path (within repo) of the token-address table source file.
TOKEN_TABLE_REL = "src/core/asm_port_token_address_table.cpp"  # nosec B105

# Matches the start of a function definition at the beginning of a line.
# Groups: (1) optional 'static ', (2) function name
FUNC_START_RE = re.compile(
    r"^(static\s+)?"
    r"(?:"
    r"void"
    r"|bool"
    r"|int"
    r"|std::u?int(?:8|16|32|64)_t"
    r"|std::string"
    r"|Inlin2Result"
    r"|InlinResult"
    r")"
    r"\s+"
    r"([A-Z][A-Z0-9_]*)"
    r"\s*\("
)

# Stub-only return patterns (body contains nothing else meaningful).
_STUB_RETURN_RE = re.compile(
    r"^\s*(?:"
    r"return\s+(?:0|false|nullptr|\{\})\s*;"
    r"|"
    r"\(void\)\s*[^;]+;"
    r")\s*$",
    re.MULTILINE,
)


# ---------------------------------------------------------------------------
# Pre-pass: compute per-line anonymous-namespace state
# ---------------------------------------------------------------------------


def _compute_anon_states(lines: list[str]) -> list[bool]:
    """
    Return a list of booleans, one per line, indicating whether that line
    falls inside an anonymous `namespace { }` block.

    Named namespaces (`namespace foo {`) do not count; only bare
    `namespace {` introduces an anonymous namespace.
    """
    states: list[bool] = []
    brace_depth: int = 0
    # Stack of brace depths at which anonymous namespaces were entered.
    # We record depth *before* the opening `{` of the namespace statement.
    anon_entry_depths: list[int] = []

    for line in lines:
        # Determine current state before processing braces on this line.
        in_anon = len(anon_entry_depths) > 0
        states.append(in_anon)

        # Check if this line opens an anonymous namespace.
        # `namespace {` with optional whitespace between keyword and `{`.
        if re.search(r"\bnamespace\s*\{", line):
            anon_entry_depths.append(brace_depth)

        # Update brace depth for all `{` and `}` on the line.
        for ch in line:
            if ch == "{":
                brace_depth += 1
            elif ch == "}":
                brace_depth -= 1
                # Check if we've just closed an anonymous namespace.
                if anon_entry_depths and brace_depth == anon_entry_depths[-1]:
                    anon_entry_depths.pop()

    return states


# ---------------------------------------------------------------------------
# Body extraction
# ---------------------------------------------------------------------------


def _read_body(lines: list[str], start_line: int) -> tuple[str, int]:
    """Read the complete function body starting from *start_line*.

    Scan forward from *start_line* (0-based) to find the opening `{`
    and collect lines up to the matching `}`.

    Returns (body_text, last_line_index) where last_line_index is 0-based.
    Raises ValueError("declaration") if a `;` is found before any `{`
    (i.e. the line is a forward declaration, not a definition).
    Raises ValueError("unterminated") if the body is never closed.
    """
    body_parts: list[str] = []
    depth = 0
    found_open = False

    for i in range(start_line, len(lines)):
        line = lines[i]
        body_parts.append(line)

        for ch in line:
            if ch == "{":
                found_open = True
                depth += 1
            elif ch == "}":
                depth -= 1
            elif ch == ";" and not found_open:
                raise ValueError("declaration")

        if found_open and depth == 0:
            return "".join(body_parts), i

    raise ValueError("unterminated")


# ---------------------------------------------------------------------------
# Stub classification
# ---------------------------------------------------------------------------


def _strip_line_comments(text: str) -> str:
    """Remove // … single-line comments from text."""
    return re.sub(r"//[^\n]*", "", text)


def _is_stub(body: str) -> bool:
    """Return True if the function body looks like an unimplemented stub."""
    # Rule 1: explicit TODO marker anywhere in the body.
    if "TODO(asm-port)" in body:
        return True

    # Extract inner content between the outermost { }.
    open_idx = body.index("{")
    close_idx = body.rindex("}")
    inner = body[open_idx + 1 : close_idx]

    # Rule 2: empty body after stripping whitespace and comments.
    inner_stripped = _strip_line_comments(inner).strip()
    if not inner_stripped:
        return True

    # Rule 3: body contains only stub-return / (void) suppression lines.
    lines = [ln.strip() for ln in inner_stripped.splitlines() if ln.strip()]
    if all(_STUB_RETURN_RE.match(ln) for ln in lines):
        return True

    return False


# ---------------------------------------------------------------------------
# Main extraction
# ---------------------------------------------------------------------------


def extract_functions(src_root: Path) -> list[tuple[str, int, str, str]]:
    """
    Scan all .cpp files under *src_root* and return a list of
    (rel_path, 1-based-line, func_name, status) tuples.
    """
    results: list[tuple[str, int, str, str]] = []

    for cpp_file in sorted(src_root.rglob("*.cpp")):
        rel = str(cpp_file.relative_to(REPO_ROOT)).replace("\\", "/")
        is_token_table = rel == TOKEN_TABLE_REL

        try:
            text = cpp_file.read_text(encoding="utf-8")
        except OSError as exc:
            print(f"warning: cannot read {cpp_file}: {exc}", file=sys.stderr)
            continue

        lines = text.splitlines(keepends=True)
        anon_states = _compute_anon_states(lines)

        i = 0
        while i < len(lines):
            line = lines[i]
            m = FUNC_START_RE.match(line)
            if m:
                is_static = bool(m.group(1))
                func_name = m.group(2)
                in_anon = anon_states[i]

                # Apply inclusion filter.
                if is_token_table:
                    # Include all (static or non-static) uppercase functions.
                    pass
                else:
                    if is_static:
                        # Static functions in non-table files are local helpers.
                        i += 1
                        continue
                    if in_anon and not func_name.startswith("MON_"):
                        # Anonymous-namespace functions are internal, unless
                        # they carry the MON_ prefix (real monitor functions).
                        i += 1
                        continue

                # Try to read the body; skip forward declarations.
                try:
                    body, end_line = _read_body(lines, i)
                except ValueError:
                    i += 1
                    continue

                status = "stub" if _is_stub(body) else "real"
                results.append((rel, i + 1, func_name, status))

                # Jump past the consumed body.
                i = end_line + 1
                continue

            i += 1

    return results


# ---------------------------------------------------------------------------
# Markdown output
# ---------------------------------------------------------------------------

_HEADER = """\
# Applesoft Function Cross-Reference

**Status Legend:**

- **real**: Fully implemented with ROM semantics
- **stub**: Placeholder/TODO; porting not yet complete

"""

_TABLE_HEADER = (
    "| {:<19} | {:<41} | {:<4} | {:<6} |\n" "| {:<19} | {:<41} | {:<4} | {:<6} |\n"
).format(
    "Function",
    "File",
    "Line",
    "Status",
    "-" * 19,
    "-" * 41,
    "-" * 4,
    "-" * 6,
)


def _escape_md(name: str) -> str:
    """Escape Markdown special characters in a function name."""
    return name.replace("_", r"\_") if name.endswith("_") else name


def write_table(entries: list[tuple[str, int, str, str]], output: Path) -> None:
    """Sort entries by function name and write the Markdown table."""
    entries_sorted = sorted(entries, key=lambda e: e[2].upper())

    lines: list[str] = [_HEADER, _TABLE_HEADER]
    for rel, lineno, name, status in entries_sorted:
        md_name = _escape_md(name)
        lines.append(f"| {md_name:<19} | {rel:<41} | {lineno:<4} | {status:<6} |\n")

    output.write_text("".join(lines), encoding="utf-8")
    print(f"wrote {len(entries_sorted)} entries to {output}")


# ---------------------------------------------------------------------------
# Entry point
# ---------------------------------------------------------------------------


def main() -> None:
    entries = extract_functions(SRC_ROOT)
    if not entries:
        print("error: no functions found", file=sys.stderr)
        sys.exit(1)
    write_table(entries, OUTPUT)


if __name__ == "__main__":
    main()
