#!/usr/bin/env python3
"""Generate a symbol table annotated with implementation claims.

The script performs three steps:
1) Parse all *.sym files and keep only symbols whose type field is 0x0002.
2) Build a per-module symbol list with synthetic [start]=0x0000 and
   [end]=0xFFFF entries, sorted by value.
3) Scan C++ sources for `AS_Labels:` / `MON_Labels:` comments, map each range
   to the next function definition, and annotate symbols covered by that range.

Additionally, it writes a per-claim log showing which claim was scanned for
which function and how (or whether) that claim mapped to symbol labels.
"""

from __future__ import annotations

import argparse
import csv
import re
from collections import defaultdict
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

REPO_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_SYM_ROOT = (
    REPO_ROOT / "SourceMaterial" / "Apple-II-Source-slim" / "src" / "system"
)
DEFAULT_SRC_ROOT = REPO_ROOT / "src"
DEFAULT_OUT = REPO_ROOT / "docs" / "symbol-implementation-map.tsv"
DEFAULT_LOG_OUT = REPO_ROOT / "docs" / "symbol-implementation-map-claims.tsv"

# A symbol declaration line in a .sym file looks like:
#   SYMBOL, 0x1234, 0, 0x0002
SYM_DECL_RE = re.compile(
    r"^\s*([^,\s][^,]*)\s*,\s*(0x[0-9a-fA-F]+)\s*,\s*[^,]*\s*,\s*(0x[0-9a-fA-F]+)\s*$"
)

# We accept labels that appear in the AS_Labels/MON_Labels comment style.
LABEL_TOKEN_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")

# Function definition heuristic: enough for this repository's free functions.
FUNC_DEF_RE = re.compile(
    r"^\s*(?:static\s+)?"
    r"(?:inline\s+)?"
    r"(?:constexpr\s+)?"
    r"[A-Za-z_][A-Za-z0-9_:<>\s*&]*\s+"
    r"([A-Za-z_][A-Za-z0-9_]*)\s*\([^;{}]*\)\s*(?:\{|$)"
)


@dataclass(frozen=True)
class Symbol:
    module: str
    name: str
    value: int
    is_synthetic: bool = False


@dataclass(frozen=True)
class LabelRangeClaim:
    file_path: Path
    line_number: int
    function_name: str
    start_label: str
    end_label: str | None
    raw_claim: str


@dataclass(frozen=True)
class LabelMatch:
    requested_label: str
    matched_label: str
    value: int


@dataclass(frozen=True)
class ClaimResolution:
    claim: LabelRangeClaim
    module: str | None
    start_match: LabelMatch | None
    end_match: LabelMatch | None
    status: str
    note: str


def module_from_sym_path(sym_path: Path, sym_root: Path) -> str:
    rel = sym_path.relative_to(sym_root)
    stem = sym_path.name
    if stem.endswith(".sym"):
        stem = stem[:-4]
    if stem.endswith(".o65"):
        stem = stem[:-4]
    parent = "" if rel.parent == Path(".") else str(rel.parent).replace("\\", "/")
    return stem if not parent else f"{parent}/{stem}"


def parse_sym_files(sym_root: Path) -> dict[str, list[Symbol]]:
    by_module: dict[str, list[Symbol]] = defaultdict(list)
    for sym_path in sorted(sym_root.rglob("*.sym")):
        module = module_from_sym_path(sym_path, sym_root)
        for raw in sym_path.read_text(encoding="utf-8", errors="replace").splitlines():
            m = SYM_DECL_RE.match(raw)
            if not m:
                continue
            name = m.group(1).strip()
            value = int(m.group(2), 16)
            sym_type = int(m.group(3), 16)
            if sym_type != 0x0002:
                continue
            by_module[module].append(Symbol(module=module, name=name, value=value))

    for module, symbols in by_module.items():
        symbols.append(
            Symbol(module=module, name="[start]", value=0x0000, is_synthetic=True)
        )
        symbols.append(
            Symbol(module=module, name="[end]", value=0xFFFF, is_synthetic=True)
        )
        symbols.sort(key=lambda s: (s.value, s.name))

    return by_module


def _next_function_name(lines: list[str], start_idx: int) -> str | None:
    for i in range(start_idx + 1, len(lines)):
        line = lines[i]
        if "AS_Labels:" in line or "MON_Labels:" in line:
            break

        m = FUNC_DEF_RE.match(line)
        if not m:
            continue

        func_name = m.group(1)
        if func_name in {"if", "for", "while", "switch", "return"}:
            continue

        # Require function-definition context (opening brace on line or shortly after).
        tail = line[m.end() :]
        if "{" in tail:
            return func_name
        for j in range(i + 1, min(i + 8, len(lines))):
            probe = lines[j].strip()
            if not probe:
                continue
            if probe.startswith("//"):
                continue
            if probe.startswith("{"):
                return func_name
            if probe.endswith(";"):
                break
            if "{" in probe:
                return func_name
            break
    return None


def _prev_function_name(lines: list[str], start_idx: int) -> str | None:
    """Search backward from start_idx to find the enclosing function definition.

    Tracks brace depth to ensure we find the function that CONTAINS start_idx,
    not a function defined before it.
    """
    brace_depth = 0

    for i in range(start_idx, -1, -1):
        line = lines[i]

        # Count braces in this line (right to left, since we're going backward).
        for ch in reversed(line):
            if ch == "}":
                brace_depth += 1
            elif ch == "{":
                if brace_depth > 0:
                    brace_depth -= 1
                else:
                    # We've found an unmatched opening brace.
                    # This might be the start of the function we're in.
                    m = FUNC_DEF_RE.match(line)
                    if m:
                        func_name = m.group(1)
                        if func_name not in {"if", "for", "while", "switch", "return"}:
                            return func_name
                    # Otherwise, keep searching backward, but now we know we're
                    # in a different function's scope.

        # If we've closed all braces at or before this line, we've exited the
        # function scope entirely; stop searching.
        if brace_depth > 0 and line.strip() and "{" not in line:
            continue

    return None


def _extract_label_tokens(label_comment: str) -> tuple[str, str | None]:
    # Normalize by removing trailing prose markers.
    text = label_comment.replace("(inclusive)", "").replace("(exclusive)", "")
    text = text.replace("(inclusive,", "(").replace("(exclusive,", "(")

    if ".." in text:
        left, right = text.split("..", 1)
        left_tokens = LABEL_TOKEN_RE.findall(left)
        right_tokens = LABEL_TOKEN_RE.findall(right)
        start = left_tokens[-1] if left_tokens else ""
        end = right_tokens[0] if right_tokens else None
        return start, end

    tokens = LABEL_TOKEN_RE.findall(text)
    return (tokens[-1] if tokens else "", None)


def scan_label_claims(src_root: Path) -> list[LabelRangeClaim]:
    claims: list[LabelRangeClaim] = []
    for cpp_path in sorted(src_root.rglob("*.cpp")):
        lines = cpp_path.read_text(encoding="utf-8", errors="replace").splitlines()
        for idx, line in enumerate(lines):
            pos = line.find("AS_Labels:")
            if pos < 0:
                pos = line.find("MON_Labels:")
            if pos < 0:
                continue

            label_comment = line[pos:].split(":", 1)[1].strip()
            start_label, end_label = _extract_label_tokens(label_comment)
            if not start_label:
                continue

            # Try forward search first (annotation before function def).
            function_name = _next_function_name(lines, idx)
            # If not found, try backward search (annotation inside function body).
            if not function_name:
                function_name = _prev_function_name(lines, idx)
            if not function_name:
                continue

            claims.append(
                LabelRangeClaim(
                    file_path=cpp_path,
                    line_number=idx + 1,
                    function_name=function_name,
                    start_label=start_label,
                    end_label=end_label,
                    raw_claim=label_comment,
                )
            )
    return claims


@dataclass(frozen=True)
class FunctionDefinition:
    file_path: Path
    line_number: int
    name: str


def scan_all_function_definitions(src_root: Path) -> list[FunctionDefinition]:
    """Scan all C++ files for function definitions (not just those with label claims)."""
    definitions: list[FunctionDefinition] = []
    for cpp_path in sorted(src_root.rglob("*.cpp")):
        lines = cpp_path.read_text(encoding="utf-8", errors="replace").splitlines()
        for idx, line in enumerate(lines):
            m = FUNC_DEF_RE.match(line)
            if not m:
                continue

            func_name = m.group(1)
            if func_name in {"if", "for", "while", "switch", "return"}:
                continue

            # Require function-definition context (opening brace on line or shortly after).
            tail = line[m.end() :]
            has_brace = "{" in tail
            if not has_brace:
                for j in range(idx + 1, min(idx + 8, len(lines))):
                    probe = lines[j].strip()
                    if not probe:
                        continue
                    if probe.startswith("//"):
                        continue
                    if probe.startswith("{"):
                        has_brace = True
                        break
                    if probe.endswith(";"):
                        break
                    if "{" in probe:
                        has_brace = True
                        break
                    break

            if has_brace:
                definitions.append(
                    FunctionDefinition(
                        file_path=cpp_path,
                        line_number=idx + 1,
                        name=func_name,
                    )
                )
    return definitions


def _label_candidates(label: str) -> Iterable[str]:
    yield label
    if label.startswith("AS_"):
        yield label[3:]
    if label.startswith("MON_"):
        yield label[4:]
    if label.endswith("_"):
        yield label[:-1]


def _build_index(
    symbols_by_module: dict[str, list[Symbol]],
) -> dict[tuple[str, str], int]:
    index: dict[tuple[str, str], int] = {}
    for module, symbols in symbols_by_module.items():
        for sym in symbols:
            index[(module, sym.name)] = sym.value
    return index


def _match_label_value(
    module: str,
    label: str,
    symbol_index: dict[tuple[str, str], int],
) -> LabelMatch | None:
    for candidate in _label_candidates(label):
        key = (module, candidate)
        if key in symbol_index:
            return LabelMatch(
                requested_label=label,
                matched_label=candidate,
                value=symbol_index[key],
            )
    return None


def resolve_claim_to_module(
    claim: LabelRangeClaim,
    symbols_by_module: dict[str, list[Symbol]],
    symbol_index: dict[tuple[str, str], int],
) -> ClaimResolution:
    # Prefer module inference from label prefix if present.
    preferred_prefix = ""
    if claim.start_label.startswith("AS_"):
        preferred_prefix = "applesoft"
    elif claim.start_label.startswith("MON_"):
        preferred_prefix = "monitor/"

    modules = list(symbols_by_module.keys())
    if preferred_prefix:
        preferred = [m for m in modules if m.startswith(preferred_prefix)]
        non_preferred = [m for m in modules if not m.startswith(preferred_prefix)]
        modules = preferred + non_preferred

    for module in modules:
        start_match = _match_label_value(module, claim.start_label, symbol_index)
        if start_match is None:
            continue

        if claim.end_label is None:
            return ClaimResolution(
                claim=claim,
                module=module,
                start_match=start_match,
                end_match=None,
                status="resolved-single",
                note="single-label-claim",
            )

        end_match = _match_label_value(module, claim.end_label, symbol_index)
        if end_match is not None:
            if end_match.value <= start_match.value:
                return ClaimResolution(
                    claim=claim,
                    module=module,
                    start_match=start_match,
                    end_match=end_match,
                    status="skipped-ambiguous-range",
                    note="end-not-greater-than-start",
                )
            return ClaimResolution(
                claim=claim,
                module=module,
                start_match=start_match,
                end_match=end_match,
                status="resolved-range",
                note="ok",
            )

    return ClaimResolution(
        claim=claim,
        module=None,
        start_match=None,
        end_match=None,
        status="unresolved",
        note="no-module-match",
    )


def apply_claims(
    symbols_by_module: dict[str, list[Symbol]],
    claims: list[LabelRangeClaim],
    all_functions: list[FunctionDefinition],
) -> tuple[dict[tuple[str, str, int], set[str]], list[ClaimResolution]]:
    implementations: dict[tuple[str, str, int], set[str]] = defaultdict(set)
    resolutions: list[ClaimResolution] = []
    symbol_index = _build_index(symbols_by_module)

    # Build set of claimed function names (key: file + name combo to handle duplicates).
    claimed_function_keys = {(c.file_path, c.function_name) for c in claims}

    # Track which claims we've processed.
    for claim in claims:
        resolved = resolve_claim_to_module(claim, symbols_by_module, symbol_index)
        resolutions.append(resolved)
        if not resolved.module or not resolved.start_match:
            continue

        module = resolved.module
        start_value = resolved.start_match.value
        end_value = resolved.end_match.value if resolved.end_match else None
        if resolved.status == "skipped-ambiguous-range":
            continue

        for sym in symbols_by_module[module]:
            include = False
            if end_value is None:
                include = sym.value == start_value
            else:
                include = start_value <= sym.value < end_value

            if include:
                key = (sym.module, sym.name, sym.value)
                implementations[key].add(claim.function_name)

    # Add unclaimed functions to the resolutions list.
    for func_def in all_functions:
        key = (func_def.file_path, func_def.name)
        if key not in claimed_function_keys:
            # Create a synthetic "unclaimed" resolution.
            unclaimed_claim = LabelRangeClaim(
                file_path=func_def.file_path,
                line_number=func_def.line_number,
                function_name=func_def.name,
                start_label="",
                end_label=None,
                raw_claim="",
            )
            unclaimed_resolution = ClaimResolution(
                claim=unclaimed_claim,
                module=None,
                start_match=None,
                end_match=None,
                status="unclaimed",
                note="no-label-range-claim",
            )
            resolutions.append(unclaimed_resolution)

    return implementations, resolutions


def write_output(
    out_path: Path,
    symbols_by_module: dict[str, list[Symbol]],
    implementations: dict[tuple[str, str, int], set[str]],
) -> None:
    out_path.parent.mkdir(parents=True, exist_ok=True)
    with out_path.open("w", encoding="utf-8", newline="") as f:
        writer = csv.writer(f, delimiter="\t")
        writer.writerow(["module", "symbol", "value", "implemented_by"])
        for module in sorted(symbols_by_module.keys()):
            for sym in symbols_by_module[module]:
                key = (sym.module, sym.name, sym.value)
                funcs = sorted(implementations.get(key, set()))
                writer.writerow(
                    [module, sym.name, f"0x{sym.value:04x}", ", ".join(funcs)]
                )


def write_claim_log(out_path: Path, resolutions: list[ClaimResolution]) -> None:
    out_path.parent.mkdir(parents=True, exist_ok=True)
    with out_path.open("w", encoding="utf-8", newline="") as f:
        writer = csv.writer(f, delimiter="\t")
        writer.writerow(
            [
                "function",
                "file",
                "line",
                "claim_raw",
                "claim_start_label",
                "claim_end_label",
                "mapped_module",
                "mapped_start_label",
                "mapped_start_value",
                "mapped_end_label",
                "mapped_end_value",
                "status",
                "note",
            ]
        )

        for res in resolutions:
            claim = res.claim
            mapped_start_label = (
                res.start_match.matched_label if res.start_match else ""
            )
            mapped_start_value = (
                f"0x{res.start_match.value:04x}" if res.start_match else ""
            )
            mapped_end_label = res.end_match.matched_label if res.end_match else ""
            mapped_end_value = f"0x{res.end_match.value:04x}" if res.end_match else ""
            writer.writerow(
                [
                    claim.function_name,
                    str(claim.file_path.relative_to(REPO_ROOT)).replace("\\", "/"),
                    claim.line_number,
                    claim.raw_claim,
                    claim.start_label,
                    claim.end_label or "",
                    res.module or "",
                    mapped_start_label,
                    mapped_start_value,
                    mapped_end_label,
                    mapped_end_value,
                    res.status,
                    res.note,
                ]
            )


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--sym-root",
        type=Path,
        default=DEFAULT_SYM_ROOT,
        help="Root directory to scan for *.sym files",
    )
    parser.add_argument(
        "--src-root",
        type=Path,
        default=DEFAULT_SRC_ROOT,
        help="Root directory to scan for C++ source files",
    )
    parser.add_argument(
        "--out",
        type=Path,
        default=DEFAULT_OUT,
        help="Output TSV path",
    )
    parser.add_argument(
        "--log-out",
        type=Path,
        default=DEFAULT_LOG_OUT,
        help="Per-claim mapping log TSV path",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    symbols_by_module = parse_sym_files(args.sym_root)
    claims = scan_label_claims(args.src_root)
    all_functions = scan_all_function_definitions(args.src_root)
    implementations, resolutions = apply_claims(
        symbols_by_module, claims, all_functions
    )
    write_output(args.out, symbols_by_module, implementations)
    write_claim_log(args.log_out, resolutions)
    print(f"Wrote {args.out}")
    print(f"Wrote {args.log_out}")
    print(f"Modules: {len(symbols_by_module)}")
    print(f"Label range claims: {len(claims)}")
    print(f"Total functions: {len(all_functions)}")
    print(
        f"Unclaimed functions: {len([r for r in resolutions if r.status == 'unclaimed'])}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
