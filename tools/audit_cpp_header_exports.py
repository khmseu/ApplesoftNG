#!/usr/bin/env python3
"""Audit free-function export ownership between src/*.cpp and include/*.hpp.

Checks:
1) Every .cpp file (except explicitly ignored ones) has a corresponding .hpp.
2) Top-level, non-anonymous-namespace function definitions in each .cpp are
   declared in the corresponding .hpp.
3) Those function declarations do not appear in other headers.

This is a lightweight textual audit intended for iterative cleanup.
"""

from __future__ import annotations

import re
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Set, Tuple

ROOT = Path(__file__).resolve().parents[1]
SRC = ROOT / "src"
INCLUDE = ROOT / "include"

IGNORED_CPP = {
    "main.cpp",
}

# Single-line declaration matcher.
DECL_RE = re.compile(
    r"^\s*(?:extern\s+)?(?:inline\s+)?(?:constexpr\s+)?"
    r"[A-Za-z_:\d][A-Za-z0-9_:\d<>,\s\*&~]*"
    r"\b([A-Za-z_][A-Za-z0-9_]*)\s*\([^;{}]*\)\s*;\s*$"
)

# Single-line definition starter matcher.
DEF_HEAD_RE = re.compile(
    r"^\s*[A-Za-z_:\d][A-Za-z0-9_:\d<>,\s\*&~]*"
    r"\b(?:(?P<scope>[A-Za-z_][A-Za-z0-9_:]*)::)?"
    r"(?P<name>[A-Za-z_][A-Za-z0-9_]*)\s*\([^;{}]*\)\s*\{\s*$"
)

CLASS_OR_STRUCT_START_RE = re.compile(r"^\s*(class|struct)\b[^;]*\{\s*$")

SKIP_NAMES = {
    "if",
    "for",
    "while",
    "switch",
    "catch",
}


@dataclass(frozen=True)
class DeclLocation:
    header: Path
    line: int


def read_lines(path: Path) -> List[str]:
    return path.read_text(encoding="utf-8").splitlines()


def build_header_decl_index() -> Dict[str, List[DeclLocation]]:
    index: Dict[str, List[DeclLocation]] = {}
    for header in sorted(INCLUDE.rglob("*.hpp")):
        depth = 0
        class_stack: List[int] = []

        for line_no, line in enumerate(read_lines(header), 1):
            open_count = line.count("{")
            close_count = line.count("}")

            if CLASS_OR_STRUCT_START_RE.match(line):
                # Track class/struct scope depth so member declarations are
                # excluded from free-function ownership checks.
                class_stack.append(depth + open_count)

            in_class_scope = bool(class_stack)

            if in_class_scope:
                depth += open_count - close_count
                while class_stack and depth < class_stack[-1]:
                    class_stack.pop()
                continue

            m = DECL_RE.match(line)
            if not m:
                depth += open_count - close_count
                while class_stack and depth < class_stack[-1]:
                    class_stack.pop()
                continue
            name = m.group(1)
            index.setdefault(name, []).append(DeclLocation(header=header, line=line_no))

            depth += open_count - close_count
            while class_stack and depth < class_stack[-1]:
                class_stack.pop()
    return index


def collect_exported_defs(cpp: Path) -> Set[str]:
    """Collect top-level free-function definitions not in anonymous namespace."""
    lines = read_lines(cpp)
    names: Set[str] = set()

    in_anon = False
    anon_depth = 0

    for line in lines:
        stripped = line.strip()

        # Enter anonymous namespace blocks.
        if stripped.startswith("namespace {"):
            in_anon = True
            anon_depth = 1
            continue

        if in_anon:
            anon_depth += line.count("{")
            anon_depth -= line.count("}")
            if anon_depth <= 0:
                in_anon = False
            continue

        m = DEF_HEAD_RE.match(line)
        if not m:
            continue
        if stripped.startswith("static "):
            continue

        # Member/qualified definitions (Type::method) are out of scope.
        # DEF_HEAD_RE is intentionally broad and can greedily consume scope
        # qualifiers into the return-type segment, so also check whether the
        # function name is directly preceded by "::" in the original line.
        name_start = m.start("name")
        is_qualified_member = (
            name_start >= 2 and line[name_start - 2 : name_start] == "::"
        )
        if m.group("scope") is not None or is_qualified_member:
            continue

        name = m.group("name")
        if name in SKIP_NAMES:
            continue
        if name.startswith("~"):
            continue
        names.add(name)

    return names


def main() -> int:
    decl_index = build_header_decl_index()

    missing_pairs: List[Path] = []
    missing_decl: List[Tuple[Path, Path, str]] = []
    wrong_header_decl: List[Tuple[Path, Path, str, List[DeclLocation]]] = []

    for cpp in sorted(SRC.rglob("*.cpp")):
        rel = cpp.relative_to(SRC)
        if rel.as_posix() in IGNORED_CPP:
            continue

        owner_header = INCLUDE / rel.with_suffix(".hpp")
        if not owner_header.exists():
            missing_pairs.append(cpp)
            continue

        exported = collect_exported_defs(cpp)
        for name in sorted(exported):
            locations = decl_index.get(name, [])
            if not locations:
                missing_decl.append((cpp, owner_header, name))
                continue

            owner_hits = [loc for loc in locations if loc.header == owner_header]
            if not owner_hits:
                wrong_header_decl.append((cpp, owner_header, name, locations))
                continue

            non_owner_hits = [loc for loc in locations if loc.header != owner_header]
            if non_owner_hits:
                wrong_header_decl.append((cpp, owner_header, name, non_owner_hits))

    has_error = False

    if missing_pairs:
        has_error = True
        print("Missing corresponding headers:")
        for cpp in missing_pairs:
            rel = cpp.relative_to(ROOT)
            print(f"  - {rel}")

    if missing_decl:
        has_error = True
        print("\nMissing declarations in corresponding headers:")
        for cpp, owner_header, name in missing_decl:
            print(
                f"  - {name}: {cpp.relative_to(ROOT)} -> {owner_header.relative_to(ROOT)}"
            )

    if wrong_header_decl:
        has_error = True
        print("\nDeclarations in non-owning headers:")
        for cpp, owner_header, name, locations in wrong_header_decl:
            locs = ", ".join(
                f"{loc.header.relative_to(ROOT)}:{loc.line}" for loc in locations
            )
            print(
                f"  - {name}: owner={owner_header.relative_to(ROOT)}, "
                f"cpp={cpp.relative_to(ROOT)}, found={locs}"
            )

    if not has_error:
        print("No ownership violations found.")
        return 0

    return 1


if __name__ == "__main__":
    sys.exit(main())
