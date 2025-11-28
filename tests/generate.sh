#!/usr/bin/env bash
#set -euo pipefail

# fix relative paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR/.." || exit 1

ASM_BIN="$SCRIPT_DIR/../build/u2asm"
VM_BIN="$SCRIPT_DIR/../build/u2vm"
TEST_DIR="$SCRIPT_DIR/src"

echo "=== Generating reference test cases ==="

for src in "$TEST_DIR"/*.u2a; do
    base=$(basename "$src" .u2a)
    ref_dir="$TEST_DIR/refs/$base.ref"
    mkdir -p "$ref_dir"

    u2b_file="$ref_dir/$base.u2b.out"
    asm_stdout="$ref_dir/$base.asm.out"
    vm_stdout="$ref_dir/$base.vm.out"

    echo "--- Assembling $src ---"
    $ASM_BIN --dev "$src" "$u2b_file" > "$asm_stdout"

    echo "--- Running VM on $u2b_file ---"
    $VM_BIN --dev "$u2b_file" > "$vm_stdout"
done

echo "=== Reference generation complete ==="
