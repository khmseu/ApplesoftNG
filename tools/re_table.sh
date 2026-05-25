#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

python3 "${SCRIPT_DIR}/gen_cross_reference.py"
python3 "${SCRIPT_DIR}/gen_symbol_implementation_map.py"
