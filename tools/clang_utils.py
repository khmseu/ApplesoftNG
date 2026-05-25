#!/usr/bin/env python3
"""Shared helpers for scripts that parse C++ with libclang."""

from __future__ import annotations

import sys
from pathlib import Path
from typing import Any, Sequence


def import_clang_cindex() -> Any:
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


def configure_libclang(cindex: Any, candidates: Sequence[str]) -> None:
    """Pin bindings to a known libclang shared library if not configured."""
    if cindex.Config.library_file:
        return

    for candidate in candidates:
        if Path(candidate).is_file():
            cindex.Config.set_library_file(candidate)
            return


def resolve_path(path_str: str) -> Path:
    """Resolve a path best-effort without failing hard on missing files."""
    try:
        return Path(path_str).resolve()
    except OSError:
        return Path(path_str)


def compile_args_for_file(
    db: Any | None,
    cpp_file: Path,
    include_dir: Path,
    std: str = "c++23",
) -> list[str]:
    """Build parser args from compile_commands for a specific source file."""
    fallback = [f"-std={std}", f"-I{include_dir}"]
    if db is None:
        return fallback

    commands = db.getCompileCommands(str(cpp_file))
    if not commands:
        return fallback

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
