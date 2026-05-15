#!/usr/bin/env python3
"""
Regenerate docs/function-cross-reference.md from C++ source files.

Scans all src/**/*.cpp files for function definitions with bodies,
classifies each as 'real' or 'stub', and writes a sorted Markdown table
to docs/function-cross-reference.md.

Inclusion rules
---------------
- Every function definition that has a body `{ … }` is included,
  regardless of name, static qualifier, or enclosing namespace.
- Forward declarations (ending with `;`) are skipped.

Stub classification
-------------------
A function is classified as 'stub' if ANY of the following hold:
  1. A `TODO(asm-port)` marker appears in the comment block immediately
     before the function signature, OR anywhere inside the body.
  2. The body is empty after stripping whitespace and single-line comments.
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
    r"|[A-Za-z_][A-Za-z0-9_]*"
    r")"
    r"\s+"
    r"([A-Za-z_][A-Za-z0-9_]*)"
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


def _is_stub(body: str, preceding_comments: str = "") -> bool:
    """Return True if the function body looks like an unimplemented stub."""
    # Rule 1: explicit TODO marker in the preceding comment block or the body.
    if "TODO(asm-port)" in preceding_comments or "TODO(asm-port)" in body:
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


def _get_size(body: str) -> int:
    """Return the number of lines in the function that are not purely comment."""
    # Extract inner content between the outermost { }.
    open_idx = body.index("{")
    close_idx = body.rindex("}")
    inner = body[open_idx + 1 : close_idx]

    lines = inner.splitlines()
    count = 0
    for line in lines:
        stripped = line.strip()
        if not stripped:
            continue
        # Check if line is purely a comment
        if (
            stripped.startswith("//")
            or stripped.startswith("/*")
            and stripped.endswith("*/")
        ):
            # Note: very crude check for /* */, but mostly we use //
            continue
        # Also strip line-end comments and check if anything remains
        if _strip_line_comments(stripped).strip():
            count += 1
    return count


# ---------------------------------------------------------------------------
# Main extraction
# ---------------------------------------------------------------------------


def extract_functions(src_root: Path) -> list[tuple[str, int, str, str, int, int]]:
    """
    Scan all .cpp files under *src_root* and return a list of
    (rel_path, 1-based-line, func_name, status, size, caller_count) tuples.
    """
    results: list[tuple[str, int, str, str, int, int]] = []
    all_names: list[str] = []

    # First pass: collect all function names across all files.
    for cpp_file in sorted(src_root.rglob("*.cpp")):
        try:
            text = cpp_file.read_text(encoding="utf-8")
        except OSError:
            continue
        all_names.extend(re.findall(r"([A-Za-z_][A-Za-z0-9_]*)\s*\(", text))

    for cpp_file in sorted(src_root.rglob("*.cpp")):
        rel = str(cpp_file.relative_to(REPO_ROOT)).replace("\\", "/")

        try:
            text = cpp_file.read_text(encoding="utf-8")
        except OSError as exc:
            print(f"warning: cannot read {cpp_file}: {exc}", file=sys.stderr)
            continue

        lines = text.splitlines(keepends=True)

        i = 0
        while i < len(lines):
            line = lines[i]
            m = FUNC_START_RE.match(line)
            if m:
                func_name = m.group(2)

                # Collect consecutive comment lines immediately before this line.
                j = i - 1
                comment_parts: list[str] = []
                while j >= 0 and lines[j].lstrip().startswith("//"):
                    comment_parts.append(lines[j])
                    j -= 1
                preceding = "".join(reversed(comment_parts))

                # Try to read the body; skip forward declarations.
                try:
                    body, end_line = _read_body(lines, i)
                except ValueError:
                    i += 1
                    continue

                status = "stub" if _is_stub(body, preceding) else "real"
                size = _get_size(body)

                # Crude count of occurrences of func_name in all source files.
                # Subtract 1 because its own definition matches.
                caller_count = max(0, all_names.count(func_name) - 1)

                results.append((rel, i + 1, func_name, status, size, caller_count))

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
<!-- This file is auto-generated; do not edit manually. Regenerate with: python3 ./tools/gen_cross_reference.py -->

**Status Legend:**

- **real**: Fully implemented with ROM semantics
- **stub**: Placeholder/TODO; porting not yet complete

"""

_TABLE_HEADER = (
    "| {:<19} | {:<6} | {:<4} | {:<7} | {:<41} | {:<4} |\n"
    "| {:<19} | {:<6} | {:<4} | {:<7} | {:<41} | {:<4} |\n"
).format(
    "Function",
    "Status",
    "Size",
    "Callers",
    "File",
    "Line",
    "-" * 19,
    "-" * 6,
    "-" * 4,
    "-" * 7,
    "-" * 41,
    "-" * 4,
)


def _escape_md(name: str) -> str:
    """Escape Markdown special characters in a function name."""
    return name.replace("_", r"\_") if name.endswith("_") else name


def write_table(
    entries: list[tuple[str, int, str, str, int, int]], output: Path
) -> None:
    """Sort entries by function name and write the Markdown table."""
    entries_sorted = sorted(entries, key=lambda e: e[2].upper())

    lines: list[str] = [_HEADER, _TABLE_HEADER]
    for rel, lineno, name, status, size, callers in entries_sorted:
        md_name = _escape_md(name)
        lines.append(
            f"| {md_name:<19} | {status:<6} | {size:<4} | {callers:<7} | {rel:<41} | {lineno:<4} |\n"
        )

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
