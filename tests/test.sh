#!/usr/bin/env bash
#set -euo pipefail

# fix relative paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR/.." || exit 1

ASM_BIN="$SCRIPT_DIR/../build/u2asm"
VM_BIN="$SCRIPT_DIR/../build/u2vm"
TEST_DIR="$SCRIPT_DIR/src"

echo "=== Running regression tests ==="

failures=0

for src in "$TEST_DIR"/*.u2a; do
    base=$(basename "$src" .u2a)
    ref_dir="$TEST_DIR/refs/$base.ref"

    u2b_file="$ref_dir/$base.u2b.out"
    asm_stdout="$ref_dir/$base.asm.out"
    vm_stdout="$ref_dir/$base.vm.out"

    tmp_u2b="$TEST_DIR/$base.tmp.u2b"
    tmp_asm="$TEST_DIR/$base.tmp.asm.out"
    tmp_vm="$TEST_DIR/$base.tmp.vm.out"

    echo "--- Assembling $src ---"
    $ASM_BIN --dev "$src" "$tmp_u2b" > "$tmp_asm"

    echo "--- Checking assembler output ---"
    if ! cmp -s "$tmp_u2b" "$u2b_file"; then
        echo "!!! Binary mismatch for $base !!!"
        diff "$u2b_file" "$tmp_u2b" || true
        failures=$((failures+1))
    fi

    if ! cmp -s "$tmp_asm" "$asm_stdout"; then
        echo "!!! Assembler stdout mismatch for $base !!!"
        diff "$asm_stdout" "$tmp_asm" || true
        failures=$((failures+1))
    fi

    echo "--- Running VM ---"
    $VM_BIN --dev "$tmp_u2b" > "$tmp_vm"

    if ! cmp -s "$tmp_vm" "$vm_stdout"; then
        echo "!!! VM stdout mismatch for $base !!!"
        diff "$vm_stdout" "$tmp_vm" || true
        failures=$((failures+1))
    fi

    rm -f "$tmp_u2b" "$tmp_asm" "$tmp_vm"
done

if [ "$failures" -eq 0 ]; then
    echo "[-] All tests passed!"
else
    echo "!!! $failures tests failed. !!!"
    exit 1
fi

