#!/usr/bin/env python3
import csv
import os
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Tuple

ROOT = Path(__file__).resolve().parents[1]
BINARY = ROOT / "flower_exchange"
TEST_DIR = ROOT / "test_csvs"
EXPECTED_DIR = ROOT / "output_csvs"
ACTUAL_DIR = ROOT / "output_csvs" / "actual_test_runs"


@dataclass
class CaseResult:
    case_no: int
    run_ok: bool
    match_ok: bool
    message: str


def normalize_header(name: str) -> str:
    return "".join(ch.lower() for ch in name if ch not in {" ", "_"})


def canonicalize_row(row: Dict[str, str]) -> Dict[str, str]:
    normalized = {normalize_header(k): v.strip() for k, v in row.items()}

    order_id = normalized.get("orderid", "")
    client_order_id = normalized.get("clientorderid", normalized.get("clientorderid", ""))
    instrument = normalized.get("instrument", "")
    side = normalized.get("side", "")
    status = normalized.get("status", normalized.get("execstatus", ""))
    quantity = normalized.get("quantity", "")
    price = normalized.get("price", "")

    return {
        "OrderID": order_id,
        "ClientOrderID": client_order_id,
        "Instrument": instrument,
        "Side": side,
        "Status": status,
        "Quantity": quantity,
        "Price": price,
    }


def read_canonical_rows(csv_path: Path) -> Tuple[List[Dict[str, str]], str]:
    if not csv_path.exists():
        return [], f"missing file: {csv_path}"

    if csv_path.stat().st_size == 0:
        return [], f"empty file: {csv_path}"

    with csv_path.open("r", newline="") as f:
        reader = csv.DictReader(f)
        if reader.fieldnames is None:
            return [], f"no header row: {csv_path}"

        rows = [canonicalize_row(row) for row in reader]
        return rows, ""


def run_case(case_no: int) -> CaseResult:
    input_csv = TEST_DIR / f"Example {case_no}.csv"
    expected_csv = EXPECTED_DIR / f"Output {case_no}.csv"
    actual_csv = ACTUAL_DIR / f"Output {case_no}.csv"

    cmd = [str(BINARY), str(input_csv), str(actual_csv)]
    proc = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True)

    if proc.returncode != 0:
        message = (
            f"execution failed (exit {proc.returncode})\n"
            f"stdout: {proc.stdout.strip()}\n"
            f"stderr: {proc.stderr.strip()}"
        )
        return CaseResult(case_no, False, False, message)

    expected_rows, expected_err = read_canonical_rows(expected_csv)
    actual_rows, actual_err = read_canonical_rows(actual_csv)

    if expected_err:
        return CaseResult(
            case_no,
            True,
            False,
            f"cannot compare: {expected_err} (actual output generated at {actual_csv})",
        )

    if actual_err:
        return CaseResult(case_no, True, False, f"actual output problem: {actual_err}")

    if expected_rows != actual_rows:
        return CaseResult(
            case_no,
            True,
            False,
            f"mismatch: expected {len(expected_rows)} rows, got {len(actual_rows)} rows",
        )

    return CaseResult(case_no, True, True, "match")


def build_binary() -> Tuple[bool, str]:
    compile_cmd = [
        "g++",
        "-std=c++17",
        "-Wall",
        "-Wextra",
        "-pedantic",
        "main.cpp",
        "Exchange.cpp",
        "OrderBook.cpp",
        "OrderBookSide.cpp",
        "Order.cpp",
        "Orders.cpp",
        "-o",
        str(BINARY.name),
    ]
    proc = subprocess.run(compile_cmd, cwd=ROOT, capture_output=True, text=True)
    if proc.returncode != 0:
        return False, f"build failed\nstdout:\n{proc.stdout}\nstderr:\n{proc.stderr}"
    return True, "build succeeded"


def main() -> int:
    ACTUAL_DIR.mkdir(parents=True, exist_ok=True)

    ok, msg = build_binary()
    print(msg)
    if not ok:
        return 1

    results: List[CaseResult] = []
    for case_no in range(1, 8):
        result = run_case(case_no)
        results.append(result)

    passed = sum(1 for r in results if r.match_ok)
    failed = len(results) - passed

    print("\nCSV regression results")
    for r in results:
        status = "PASS" if r.match_ok else "FAIL"
        print(f"Case {r.case_no}: {status} - {r.message}")

    print(f"\nSummary: {passed} passed, {failed} failed")
    return 0 if failed == 0 else 2


if __name__ == "__main__":
    sys.exit(main())
