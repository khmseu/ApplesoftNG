#!/usr/bin/env python3
"""Sort stub candidates from docs/function-cross-reference.md.

Outputs the top N stub candidates ordered by:
1) Calls (Approx) descending
2) Function name ascending

Also prints a short best-candidate comment for the top entry.
"""

from __future__ import annotations

import argparse
import re
import sys
from dataclasses import dataclass
from pathlib import Path


@dataclass(frozen=True)
class Candidate:
    function: str
    status: str
    size: int
    calls: int
    file: str
    line: int


def _parse_row(line: str) -> Candidate | None:
    if not line.startswith("|"):
        return None

    # Skip separator rows such as: | ---- | ---- |
    if re.search(r"\|\s*-{2,}\s*\|", line):
        return None

    parts = [part.strip() for part in line.strip().split("|")[1:-1]]
    if len(parts) != 6:
        return None

    function, status, size_text, calls_text, file_path, line_text = parts
    if status.lower() != "stub":
        return None

    try:
        size = int(size_text)
        calls = int(calls_text)
        line_number = int(line_text)
    except ValueError:
        return None

    return Candidate(
        function=function,
        status=status,
        size=size,
        calls=calls,
        file=file_path,
        line=line_number,
    )


def load_stub_candidates(cross_reference: Path) -> list[Candidate]:
    rows = cross_reference.read_text(encoding="utf-8").splitlines()
    candidates = []
    for row in rows:
        parsed = _parse_row(row)
        if parsed is not None:
            candidates.append(parsed)
    return candidates


def main() -> int:
    parser = argparse.ArgumentParser(
        description=(
            "List top stub candidates sorted by Calls (Approx) descending, "
            "then function name ascending."
        )
    )
    parser.add_argument(
        "--cross-reference",
        default="docs/function-cross-reference.md",
        help="Path to function cross-reference markdown file.",
    )
    parser.add_argument(
        "--top",
        type=int,
        default=10,
        help="Number of top candidates to print (default: 10).",
    )
    args = parser.parse_args()

    cross_reference = Path(args.cross_reference)
    if not cross_reference.exists():
        print(f"error: file not found: {cross_reference}", file=sys.stderr)
        return 1

    candidates = load_stub_candidates(cross_reference)
    if not candidates:
        print("No stub candidates found.")
        return 0

    ordered = sorted(candidates, key=lambda item: (-item.calls, item.function))
    top_candidates = ordered[: max(args.top, 0)]

    print("Top stub candidates")
    print("-------------------")
    print("Function | Calls (Approx) | File | Line")
    for candidate in top_candidates:
        print(
            f"{candidate.function} | {candidate.calls} | "
            f"{candidate.file} | {candidate.line}"
        )

    if top_candidates:
        best = top_candidates[0]
        print()
        print(
            "Best candidate: "
            f"{best.function} (highest Calls (Approx): {best.calls}; "
            "tie-breaker is function name ascending)."
        )

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
