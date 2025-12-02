#!/usr/bin/env sh
set -e

ROOT="$(dirname "$0")"
BIN="$ROOT/minipy"

if [ ! -x "$BIN" ]; then
  echo "minipy 바이너리가 없습니다. 먼저 make를 실행하세요." >&2
  exit 1
fi

run() {
  name="$1"
  expect="$2"
  infile="$ROOT/tests/$name.py"
  if [ ! -f "$infile" ]; then
    echo "테스트 파일 없음: $infile" >&2
    exit 1
  fi
  echo "== $name (expected: $expect) =="
  "$BIN" "$infile"
  echo
}

run "test01_assign" "3"
run "test02_logic_if" "1"
run "test03_if_else" "1"
run "test04_while_sum" "12"
run "test05_for_range" "10"
run "test06_for_step" "9"
run "test07_break" "3"
run "test08_continue_for" "9"
run "test09_pointer" "15 (two lines)"
run "test10_pointer_chain" "6 5 6 5"
run "test11_func" "5"
