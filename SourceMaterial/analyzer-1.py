"""
6502 Assembly Analyzer for Apple II ROMs (xa65 output).

Parses xa65 .lst (listing) and .sym (symbol) files for a list of
modules described in modules.md and produces:
  - All labels with addresses & owning module
  - Detection of subroutine entry points (JSR / JMP / branch / jump-table)
  - Exported / imported symbols
  - Call graph
  - ROM-order listing
"""

from __future__ import annotations

import argparse
import os
import re
import sys
from dataclasses import dataclass, field
from typing import Dict, List, Optional, Set, Tuple

# ---------------------------------------------------------------------------
# 6502 mnemonic / directive tables
# ---------------------------------------------------------------------------
MNEMONICS: Set[str] = set(
    "adc and asl bcc bcs beq bit bmi bne bpl brk bvc bvs clc cld cli clv "
    "cmp cpx cpy dec dex dey eor inc inx iny jmp jsr lda ldx ldy lsr nop "
    "ora pha php pla plp rol ror rti rts sbc sec sed sei sta stx sty tax "
    "tay tsx txa txs tya".split()
)

BRANCH_MNEMONICS: Set[str] = {"bcc", "bcs", "beq", "bne", "bmi", "bpl", "bvc", "bvs"}

# xa65-style assembler directives that may emit bytes
DIRECTIVES: Set[str] = {
    ".word",
    ".byte",
    ".res",
    ".dw",
    ".db",
    ".text",
    ".asc",
    ".end",
    ".dword",
    ".dsb",
    ".bin",
    ".align",
}

# Default ROM bases for Apple II
DEFAULT_PARENT_BASES = {
    "asrom": 0xD000,
}


# ---------------------------------------------------------------------------
# Data classes
# ---------------------------------------------------------------------------
@dataclass
class Label:
    name: str
    module: str
    local_addr: int  # address within the module (T:XXXX)
    rom_addr: int  # absolute ROM address
    src_line: int  # source line number where defined
    is_jsr_target: bool = False
    is_jmp_target: bool = False
    is_branch_target: bool = False
    is_jumptable_entry: bool = False
    is_exported: bool = False
    is_imported: bool = False
    label_only: bool = False  # appears alone, may be data
    is_constant: bool = False  # LABEL=value style
    references: Set[str] = field(default_factory=set)  # callers (label names)

    def kinds(self) -> List[str]:
        k = []
        if self.is_exported:
            k.append("EXPORT")
        if self.is_imported:
            k.append("IMPORT")
        if self.is_jsr_target:
            k.append("JSR")
        if self.is_jmp_target:
            k.append("JMP")
        if self.is_branch_target:
            k.append("BRANCH")
        if self.is_jumptable_entry:
            k.append("JTABLE")
        if self.is_constant:
            k.append("CONST")
        if self.label_only:
            k.append("LABEL")
        return k


@dataclass
class Instruction:
    src_line: int
    local_addr: int
    rom_addr: int
    raw_bytes: List[str]
    label: Optional[str]  # label defined on this line (if any)
    mnemonic: Optional[str]  # lower-case opcode or '.word' etc.
    operand: str  # raw operand string, may be empty
    comment: str
    module: str


@dataclass
class Module:
    name: str
    parent: str  # "Applesoft" / "Autostart-Monitor"
    order: int  # global ROM order
    base_addr: int = 0  # set after layout
    size: int = 0
    labels: Dict[str, Label] = field(default_factory=dict)
    instructions: List[Instruction] = field(default_factory=list)
    sym_exports: Dict[str, int] = field(default_factory=dict)  # name -> addr
    sym_imports: Set[str] = field(default_factory=set)


# ---------------------------------------------------------------------------
# .sym parsing
# ---------------------------------------------------------------------------
SYM_LINE_RE = re.compile(
    r"""^\s*([A-Za-z_][\w]*)\s*,
        \s*0x([0-9a-fA-F]+)\s*,
        \s*\d+\s*,
        \s*0x([0-9a-fA-F]+)\s*$""",
    re.VERBOSE,
)


def parse_sym_file(path: str) -> Tuple[Dict[str, int], Set[str]]:
    """Return (exports{name:addr}, imports{name})."""
    exports: Dict[str, int] = {}
    imports: Set[str] = set()
    if not os.path.isfile(path):
        return exports, imports
    with open(path, "r", encoding="utf-8", errors="replace") as f:
        for line in f:
            m = SYM_LINE_RE.match(line)
            if not m:
                continue  # cross-reference body line
            name, addr_hex, flags_hex = m.group(1), m.group(2), m.group(3)
            addr = int(addr_hex, 16)
            flags = int(flags_hex, 16)
            if flags & 0x0002:
                exports[name] = addr
            else:
                imports.add(name)
    return exports, imports


# ---------------------------------------------------------------------------
# .lst parsing
# ---------------------------------------------------------------------------
LST_LINE_RE = re.compile(
    r"""^\s*(\d+)\s+                                  # line number
        T:([0-9a-fA-F]+)                              # address
        ((?:\s+[0-9a-f]{2}){0,4})                     # 0..4 hex bytes
        (?:\s{2,}|\s*$)                               # gap before rest
        (.*?)\s*$                                     # rest of the line
    """,
    re.VERBOSE,
)


def _strip_comment(s: str) -> Tuple[str, str]:
    # find ';' that is not inside a string literal (rare here)
    in_str = False
    quote = None
    for i, c in enumerate(s):
        if in_str:
            if c == quote:
                in_str = False
        else:
            if c in ('"', "'"):
                in_str = True
                quote = c
            elif c == ";":
                return s[:i].rstrip(), s[i:]
    return s.rstrip(), ""


def parse_lst_line(line: str):
    """Return dict with parsed fields, or None if line is irrelevant."""
    if not line.strip():
        return None
    m = LST_LINE_RE.match(line)
    if not m:
        return None
    lineno = int(m.group(1))
    addr = int(m.group(2), 16)
    raw_bytes = m.group(3).split()
    rest_raw = m.group(4)
    rest, comment = _strip_comment(rest_raw)
    if not rest:
        return {
            "lineno": lineno,
            "addr": addr,
            "bytes": raw_bytes,
            "label": None,
            "mnemonic": None,
            "operand": "",
            "comment": comment,
            "is_constant": False,
        }

    # constant definition LABEL=value
    if "=" in rest and not rest.lstrip().startswith("."):
        # but skip cases where '=' is inside operand (rare); only treat as
        # constant if first token contains '='
        first_tok = rest.split(None, 1)[0]
        if "=" in first_tok:
            name, _, val = first_tok.partition("=")
            return {
                "lineno": lineno,
                "addr": addr,
                "bytes": raw_bytes,
                "label": name,
                "mnemonic": None,
                "operand": val.strip(),
                "comment": comment,
                "is_constant": True,
            }

    tokens = rest.split(None, 2)
    label = None
    mnemonic = None
    operand = ""

    first = tokens[0]
    flow = first.lower()
    if first.startswith("."):
        mnemonic = flow
        operand = " ".join(tokens[1:]) if len(tokens) > 1 else ""
    elif flow in MNEMONICS:
        mnemonic = flow
        operand = " ".join(tokens[1:]) if len(tokens) > 1 else ""
    else:
        # first token is a label
        label = first
        if len(tokens) >= 2:
            second = tokens[1]
            slow = second.lower()
            if second.startswith(".") or slow in MNEMONICS:
                mnemonic = slow
                operand = tokens[2] if len(tokens) > 2 else ""
            else:
                # unknown - keep as operand text
                operand = " ".join(tokens[1:])

    return {
        "lineno": lineno,
        "addr": addr,
        "bytes": raw_bytes,
        "label": label,
        "mnemonic": mnemonic,
        "operand": operand,
        "comment": comment,
        "is_constant": False,
    }


# ---------------------------------------------------------------------------
# Operand → referenced label name(s)
# ---------------------------------------------------------------------------
LABEL_NAME_RE = re.compile(r"[A-Za-z_][\w]*")


def extract_target_label(operand: str) -> Optional[str]:
    """For jsr/jmp/branch operand, return the symbolic target name or None."""
    op = operand.strip()
    if not op:
        return None
    # indirect: (NAME)  or  (NAME,X)
    op = op.strip()
    if op.startswith("("):
        op = op.strip("()")
        op = op.split(",", 1)[0].strip()
    # strip trailing ,X / ,Y
    op = re.split(r",\s*[xXyY]\s*$", op)[0].strip()
    # numeric / immediate / hex / binary / dec → not a label
    if op[:1] in ("$", "#", "%"):
        return None
    if op[:1].isdigit():
        return None
    # arithmetic expression: take first identifier
    m = LABEL_NAME_RE.match(op)
    if not m:
        return None
    return m.group(0)


def extract_word_targets(operand: str) -> List[str]:
    """For .word LABEL-1, LABEL2-1  → list of label names."""
    out: List[str] = []
    for piece in operand.split(","):
        piece = piece.strip()
        if not piece:
            continue
        if piece[:1] in ("$", "#", "%"):
            continue
        if piece[:1].isdigit():
            continue
        m = LABEL_NAME_RE.match(piece)
        if m:
            out.append(m.group(0))
    return out


# ---------------------------------------------------------------------------
# Module-list parsing
# ---------------------------------------------------------------------------
def parse_modules_md(path: str) -> List[Tuple[str, str]]:
    """Return list of (parent, module_name) in ROM order."""
    out: List[Tuple[str, str]] = []
    parent = None
    with open(path, "r", encoding="utf-8") as f:
        for line in f:
            s = line.rstrip()
            m_par = re.match(r"^\s*-\s*\[([^\]]+)\]", s)
            m_sub = re.match(r"^\s{2,}-\s*([A-Za-z_][\w-]*)", s)
            if m_par:
                parent = m_par.group(1).strip()
            elif m_sub and parent is not None:
                out.append((parent, m_sub.group(1).strip()))
    return out


# ---------------------------------------------------------------------------
# Module parsing (combine lst + sym)
# ---------------------------------------------------------------------------
def load_module(name: str, parent: str, order: int, data_dir: str) -> Module:
    mod = Module(name=name, parent=parent, order=order)
    lst_path = os.path.join(data_dir, f"{name}.lst")
    sym_path = os.path.join(data_dir, f"{name}.sym")

    # symbols
    exports, imports = parse_sym_file(sym_path)
    mod.sym_exports = exports
    mod.sym_imports = imports

    # listing
    if not os.path.isfile(lst_path):
        return mod

    max_end = 0
    with open(lst_path, "r", encoding="utf-8", errors="replace") as f:
        for line in f:
            parsed = parse_lst_line(line)
            if parsed is None:
                continue
            ins = Instruction(
                src_line=parsed["lineno"],
                local_addr=parsed["addr"],
                rom_addr=parsed["addr"],  # patched later
                raw_bytes=parsed["bytes"],
                label=parsed["label"],
                mnemonic=parsed["mnemonic"],
                operand=parsed["operand"],
                comment=parsed["comment"],
                module=name,
            )
            mod.instructions.append(ins)
            if parsed["bytes"]:
                end = parsed["addr"] + len(parsed["bytes"])
                if end > max_end:
                    max_end = end

            # collect label
            if parsed["label"]:
                lname = parsed["label"]
                if lname not in mod.labels:
                    mod.labels[lname] = Label(
                        name=lname,
                        module=name,
                        local_addr=parsed["addr"],
                        rom_addr=parsed["addr"],
                        src_line=parsed["lineno"],
                        label_only=(
                            parsed["mnemonic"] is None and not parsed["is_constant"]
                        ),
                        is_constant=parsed["is_constant"],
                    )
    mod.size = max_end
    return mod


# ---------------------------------------------------------------------------
# Layout (assign rom_addr to each module + every label / instruction)
# ---------------------------------------------------------------------------
def layout_modules(modules: List[Module], parent_bases: Dict[str, int]) -> None:
    cursors: Dict[str, int] = {
        p: parent_bases.get(p, 0) for p in {m.parent for m in modules}
    }
    for mod in modules:
        base = cursors.get(mod.parent, 0)
        mod.base_addr = base
        cursors[mod.parent] = base + mod.size
        for lab in mod.labels.values():
            lab.rom_addr = base + lab.local_addr
        for ins in mod.instructions:
            ins.rom_addr = base + ins.local_addr


# ---------------------------------------------------------------------------
# Cross-module analysis
# ---------------------------------------------------------------------------
class Analyzer:
    def __init__(self, modules: List[Module]):
        self.modules = modules
        self.global_labels: Dict[str, Label] = {}
        for mod in modules:
            for lab in mod.labels.values():
                self.global_labels.setdefault(lab.name, lab)
                # mark export/import flags from sym files
                if lab.name in mod.sym_exports:
                    lab.is_exported = True
        # attach imports as "phantom" entries on modules but don't promote
        for mod in modules:
            for imp in mod.sym_imports:
                if imp in self.global_labels:
                    self.global_labels[imp].is_imported = True

    def analyze_flow(self) -> None:
        """Scan all instructions, mark JSR/JMP/branch/jump-table targets."""
        for mod in self.modules:
            for ins in mod.instructions:
                if not ins.mnemonic:
                    continue
                m = ins.mnemonic

                # current "from" label = nearest preceding labelled instruction
                from_label = self._enclosing_label(mod, ins)

                if m == "jsr":
                    tgt = extract_target_label(ins.operand)
                    self._mark(tgt, "jsr", from_label)
                elif m == "jmp":
                    tgt = extract_target_label(ins.operand)
                    self._mark(tgt, "jmp", from_label)
                elif m in BRANCH_MNEMONICS:
                    tgt = extract_target_label(ins.operand)
                    self._mark(tgt, "branch", from_label)
                elif m == ".word":
                    for tgt in extract_word_targets(ins.operand):
                        self._mark(tgt, "jtable", from_label)

    def _enclosing_label(self, mod: Module, ins: Instruction) -> Optional[str]:
        """Return the most recent label in this module at or before ins."""
        # quick linear search backward isn't needed; we can use ins.label or
        # traverse instructions earlier.  Pre-computing is faster for big
        # modules.
        cache = getattr(mod, "_label_cache", None)
        if cache is None:
            cache = {}
            cur = None
            for it in mod.instructions:
                if it.label and not it.label.startswith("."):
                    cur = it.label
                cache[id(it)] = cur
            mod._label_cache = cache
        return cache.get(id(ins))

    def _mark(self, name: Optional[str], kind: str, src_label: Optional[str]) -> None:
        if not name or name not in self.global_labels:
            return
        lab = self.global_labels[name]
        if kind == "jsr":
            lab.is_jsr_target = True
        elif kind == "jmp":
            lab.is_jmp_target = True
        elif kind == "branch":
            lab.is_branch_target = True
        elif kind == "jtable":
            lab.is_jumptable_entry = True
        if src_label:
            lab.references.add(src_label)


# ---------------------------------------------------------------------------
# Reporting
# ---------------------------------------------------------------------------
# ---------------------------------------------------------------------------
# Combined .lst writer  (rewrites T:XXXX to absolute ROM addresses)
# ---------------------------------------------------------------------------
# Match an xa65 listing line.  We only need to capture / rewrite the address.
#   group(1) = leading "  6396 " (line-num + spaces)
#   group(2) = 4..6 hex digits of T:XXXX
#   group(3) = everything after the address
COMBINED_LINE_RE = re.compile(r"^(\s*\d+\s+T:)([0-9a-fA-F]+)(.*)$")


def write_combined_lst(modules: List[Module], data_dir: str, out_path: str) -> None:
    """Concatenate every module's .lst into one file in ROM order, with each
    'T:XXXX' rewritten to the absolute ROM address used by the report."""
    total_lines = 0
    with open(out_path, "w", encoding="utf-8") as out:
        out.write(
            "; ============================================================\n"
            "; Combined Apple II ROM listing\n"
            "; Addresses are absolute (T:XXXX rewritten from layout).\n"
            "; ============================================================\n"
            "\n"
        )
        for mod in modules:
            lst_path = os.path.join(data_dir, f"{mod.name}.lst")
            if not os.path.isfile(lst_path):
                continue
            out.write(
                f"\n; ----------------------------------------------------\n"
                f";  Module: {mod.parent} :: {mod.name}\n"
                f";  Base : ${mod.base_addr:04X}    "
                f"Size: ${mod.size:04X}    "
                f"End : ${mod.base_addr + mod.size:04X}\n"
                f"; ----------------------------------------------------\n"
            )
            base = mod.base_addr
            with open(lst_path, "r", encoding="utf-8", errors="replace") as f:
                for line in f:
                    m = COMBINED_LINE_RE.match(line)
                    if m:
                        rel = int(m.group(2), 16)
                        absolute = base + rel
                        # Use 4 hex digits where possible, else widen
                        if absolute <= 0xFFFF:
                            addr_str = f"{absolute:04x}"
                        else:
                            addr_str = f"{absolute:06x}"
                        line = f"{m.group(1)}{addr_str}{m.group(3)}\n"
                    out.write(line)
                    total_lines += 1
    print(
        f"Combined listing written to {out_path} "
        f"({total_lines} lines from {len(modules)} modules)"
    )


def _hdr(title: str) -> str:
    bar = "=" * len(title)
    return f"\n{title}\n{bar}\n"


def write_report(modules: List[Module], analyzer: Analyzer, out_stream) -> None:
    write = out_stream.write
    write("6502 Assembly Analyzer  -  Apple II ROM report\n")
    write("=" * 60 + "\n")

    # 1. Exported functions
    write(_hdr("1. EXPORTED FUNCTIONS (from .sym files)"))
    for mod in modules:
        if not mod.sym_exports:
            continue
        write(f"\n[{mod.parent} :: {mod.name}]   (base ${mod.base_addr:04X})\n")
        for name, addr in sorted(mod.sym_exports.items(), key=lambda x: x[1]):
            lab = mod.labels.get(name)
            full = mod.base_addr + addr
            kinds = ",".join(lab.kinds()) if lab else ""
            write(f"  ${full:04X}  {name:<24} {kinds}\n")

    # 2. JSR-discovered subroutines
    jsr_labels = [
        label for label in analyzer.global_labels.values() if label.is_jsr_target
    ]
    write(_hdr(f"2. SUBROUTINES DISCOVERED VIA JSR ({len(jsr_labels)})"))
    for lab in sorted(jsr_labels, key=lambda label: label.rom_addr):
        write(
            f"  ${lab.rom_addr:04X}  {lab.name:<24} "
            f"[{lab.module}]  callers={len(lab.references)}\n"
        )

    # 3. JMP-discovered labels
    jmp_labels = [
        label
        for label in analyzer.global_labels.values()
        if label.is_jmp_target and not label.is_jsr_target
    ]
    write(_hdr(f"3. JMP-ONLY TARGETS ({len(jmp_labels)})"))
    for lab in sorted(jmp_labels, key=lambda label: label.rom_addr):
        write(f"  ${lab.rom_addr:04X}  {lab.name:<24} [{lab.module}]\n")

    # 4. Jump-table entries
    jt = [
        label for label in analyzer.global_labels.values() if label.is_jumptable_entry
    ]
    write(_hdr(f"4. JUMP-TABLE ENTRIES ({len(jt)})"))
    for lab in sorted(jt, key=lambda label: label.rom_addr):
        write(f"  ${lab.rom_addr:04X}  {lab.name:<24} [{lab.module}]\n")

    # 5. ROM order
    write(_hdr("5. ROM ORDER (all labels by module then address)"))
    for mod in modules:
        if not mod.labels and not mod.size:
            continue
        write(
            f"\n--- {mod.parent} :: {mod.name}   "
            f"base=${mod.base_addr:04X}  size=${mod.size:04X} ---\n"
        )
        labs = sorted(
            mod.labels.values(), key=lambda label: (label.local_addr, label.src_line)
        )
        for label in labs:
            write(
                f"  ${label.rom_addr:04X}  {label.name:<26} "
                f"[{','.join(label.kinds()) or '-'}]\n"
            )

    # 6. Call graph (compact)
    write(_hdr("6. CALL GRAPH (callee  <- callers)"))
    callees = sorted(analyzer.global_labels.values(), key=lambda label: label.rom_addr)
    for lab in callees:
        if not lab.references:
            continue
        callers = ", ".join(sorted(lab.references))
        write(f"  ${lab.rom_addr:04X} {lab.name}  <-  {callers}\n")


# ---------------------------------------------------------------------------
# Top-level driver
# ---------------------------------------------------------------------------
def run(
    data_dir: str,
    modules_md: str,
    out_path: Optional[str],
    combined_lst: Optional[str] = None,
) -> None:
    mod_specs = parse_modules_md(modules_md)
    if not mod_specs:
        raise SystemExit(f"No modules found in {modules_md}")

    modules: List[Module] = []
    for i, (parent, name) in enumerate(mod_specs):
        modules.append(load_module(name, parent, i, data_dir))

    layout_modules(modules, DEFAULT_PARENT_BASES)
    analyzer = Analyzer(modules)
    analyzer.analyze_flow()

    if out_path:
        with open(out_path, "w", encoding="utf-8") as f:
            write_report(modules, analyzer, f)
        print(f"Report written to {out_path}")
    else:
        write_report(modules, analyzer, sys.stdout)

    if combined_lst:
        write_combined_lst(modules, data_dir, combined_lst)


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument(
        "--data-dir", default=".", help="Directory containing .lst and .sym files"
    )
    ap.add_argument("--modules", default="modules.md", help="Path to modules.md")
    ap.add_argument(
        "-o", "--output", default=None, help="Write report to file instead of stdout"
    )
    ap.add_argument(
        "--combined-lst",
        default=None,
        help="Write a combined .lst with absolute addresses",
    )
    args = ap.parse_args()
    run(args.data_dir, args.modules, args.output, args.combined_lst)


if __name__ == "__main__":
    main()
