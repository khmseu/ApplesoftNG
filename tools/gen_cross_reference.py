#!/usr/bin/env python3
"""
Regenerate docs/function-cross-reference.md from C++ source files.

Uses libclang (python bindings) with compile_commands.json to discover function
definitions, then classifies each as 'real' or 'stub', and writes a sorted
Markdown table.

This avoids brittle return-type regex parsing and correctly handles templated
and scoped return types (for example ApplesoftDualPointer<const std::uint8_t>).

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
from typing import Any


def _import_clang_cindex():
    """Import clang.cindex with Ubuntu dist-packages fallback for venvs."""
    try:
        from clang import cindex as _cindex  # type: ignore

        return _cindex
    except ModuleNotFoundError:
        pass

    major = sys.version_info.major
    minor = sys.version_info.minor
    candidates = [
        "/usr/lib/python3/dist-packages",
        f"/usr/lib/python{major}/dist-packages",
        f"/usr/lib/python{major}.{minor}/dist-packages",
    ]
    for candidate in candidates:
        if Path(candidate).is_dir() and candidate not in sys.path:
            sys.path.append(candidate)

    try:
        from clang import cindex as _cindex  # type: ignore

        return _cindex
    except ModuleNotFoundError as exc:
        raise SystemExit(
            "error: python clang bindings not found.\n"
            "Install one of:\n"
            "  - apt: python3-clang-18\n"
            "  - venv: pip install clang\n"
            "Or run this script with system python outside the venv."
        ) from exc


cindex = _import_clang_cindex()


def _configure_libclang() -> None:
    """Pin bindings to libclang-18 to avoid mixed-version runtime issues."""
    if cindex.Config.library_file:
        return

    candidates = [
        "/usr/lib/llvm-18/lib/libclang.so.1",
        "/usr/lib/llvm-18/lib/libclang.so",
        "/usr/lib/x86_64-linux-gnu/libclang-18.so.1",
    ]
    for candidate in candidates:
        if Path(candidate).is_file():
            cindex.Config.set_library_file(candidate)
            return


_configure_libclang()

# ---------------------------------------------------------------------------
# Configuration
# ---------------------------------------------------------------------------

REPO_ROOT = Path(__file__).resolve().parent.parent
SRC_ROOT = REPO_ROOT / "src"
OUTPUT = REPO_ROOT / "docs" / "function-cross-reference.md"
COMPILE_COMMANDS_DIR = REPO_ROOT / "build"

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


def _collect_preceding_comments(lines: list[str], line_index: int) -> str:
    """Collect consecutive // comment lines immediately above line_index."""
    j = line_index - 1
    comment_parts: list[str] = []
    while j >= 0 and lines[j].lstrip().startswith("//"):
        comment_parts.append(lines[j])
        j -= 1
    return "".join(reversed(comment_parts))


# ---------------------------------------------------------------------------
# Clang extraction
# ---------------------------------------------------------------------------


def _resolve(path_str: str) -> Path:
    """Resolve a path best-effort without failing hard on missing files."""
    try:
        return Path(path_str).resolve()
    except OSError:
        return Path(path_str)


def _compile_args_for_file(db: Any | None, cpp_file: Path) -> list[str]:
    """Build parser args from compile_commands for a specific source file."""
    if db is None:
        return ["-std=c++23", f"-I{REPO_ROOT / 'include'}"]

    commands = db.getCompileCommands(str(cpp_file))
    if not commands:
        return ["-std=c++23", f"-I{REPO_ROOT / 'include'}"]

    command = commands[0]
    args = list(command.arguments)
    if args:
        args = args[1:]  # Drop compiler executable.

    filtered: list[str] = []
    skip_next = False
    source = str(cpp_file)

    for arg in args:
        if skip_next:
            skip_next = False
            continue

        if arg in {"-c", "-o", "-MF", "-MT", "-MQ"}:
            skip_next = True
            continue

        if arg == source:
            continue

        filtered.append(arg)

    return filtered


def _function_defs_via_clang(cpp_file: Path) -> list[tuple[str, int]]:
    """Return (function_name, 1-based line) definitions discovered by libclang."""
    db: Any | None = None
    try:
        db = cindex.CompilationDatabase.fromDirectory(str(COMPILE_COMMANDS_DIR))
    except Exception:
        db = None

    args = _compile_args_for_file(db, cpp_file)

    index = cindex.Index.create()
    try:
        tu = index.parse(str(cpp_file), args=args)
    except cindex.TranslationUnitLoadError as exc:
        print(f"warning: cannot parse {cpp_file}: {exc}", file=sys.stderr)
        return []

    resolved_source = _resolve(str(cpp_file))
    seen: set[tuple[str, int]] = set()
    found: list[tuple[str, int]] = []

    for cursor in tu.cursor.walk_preorder():
        try:
            kind = cursor.kind
        except ValueError:
            continue

        if kind != cindex.CursorKind.FUNCTION_DECL:
            continue
        if not cursor.is_definition():
            continue

        loc = cursor.location
        if loc is None or loc.file is None:
            continue

        if _resolve(loc.file.name) != resolved_source:
            continue

        if loc.line <= 0:
            continue

        key = (cursor.spelling, loc.line)
        if key in seen:
            continue

        seen.add(key)
        found.append(key)

    return found


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

    # First pass: collect all call-like tokens for crude caller counts.
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
        defs = _function_defs_via_clang(cpp_file)

        for name, lineno in defs:
            line_index = lineno - 1
            if line_index < 0 or line_index >= len(lines):
                continue

            # Confirm this is a definition with a body and gather exact body span.
            try:
                body, _ = _read_body(lines, line_index)
            except ValueError:
                continue

            preceding = _collect_preceding_comments(lines, line_index)
            status = "stub" if _is_stub(body, preceding) else "real"
            size = _get_size(body)
            caller_count = max(0, all_names.count(name) - 1)

            results.append((rel, lineno, name, status, size, caller_count))

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
