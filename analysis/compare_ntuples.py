"""Check that two run_scan.sh outputs contain identical events (same seeds).

    python compare_ntuples.py <scan-dir-A> <scan-dir-B> <n-runs>

Events are matched by eventID (the thread that processed an event can differ
between two multi-threaded runs, the event itself cannot).
"""
import glob
import sys

import numpy as np
import uproot


def load(d, run):
    parts = []
    for fn in sorted(glob.glob(f"{d}/output/*MeV_t*-{run}.root")):
        with uproot.open(fn) as f:
            if "hits" in f and f["hits"].num_entries:
                parts.append(f["hits"].arrays(library="np"))
    data = {k: np.concatenate([p[k] for p in parts]) for k in parts[0]}
    order = np.argsort(data["eventID"])
    return {k: v[order] for k, v in data.items()}


def main(a, b, nruns):
    ok = True
    for run in range(nruns):
        A, B = load(a, run), load(b, run)
        if A.keys() != B.keys():
            print(f"run {run}: different columns {set(A) ^ set(B)}")
            ok = False
            continue
        bad = [k for k in A if A[k].shape != B[k].shape
               or not np.array_equal(A[k], B[k], equal_nan=True)]
        n = len(A["eventID"])
        print(f"run {run}: {n} events, {len(A)} columns, "
              + ("IDENTICAL" if not bad else f"DIFFERENT in {bad}"))
        ok &= not bad
    print("ALL IDENTICAL" if ok else "DIFFERENCES FOUND")
    return 0 if ok else 1


if __name__ == "__main__":
    sys.exit(main(sys.argv[1], sys.argv[2], int(sys.argv[3])))
