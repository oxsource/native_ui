#!/usr/bin/env python3
"""Compare two raw RGB (rgb24) frames.

Usage: pixel_diff.py <A.rgb> <B.rgb> <WIDTH> <HEIGHT> [THRESHOLD]

Reads WIDTH*HEIGHT*3 bytes from each file, computes the max and mean absolute
difference, prints them, and exits 0 if the mean is <= THRESHOLD (default 8.0).
The threshold tolerates lossy (e.g. H.264) encoding of the compared frame.
"""
import sys


def main() -> int:
    if len(sys.argv) < 5:
        print(__doc__, file=sys.stderr)
        return 2
    a_path, b_path = sys.argv[1], sys.argv[2]
    w, h = int(sys.argv[3]), int(sys.argv[4])
    threshold = float(sys.argv[5]) if len(sys.argv) > 5 else 8.0
    n = w * h * 3

    a = open(a_path, "rb").read()[:n]
    b = open(b_path, "rb").read()[:n]
    if len(a) != n or len(b) != n:
        print(f"ERROR: size mismatch a={len(a)} b={len(b)} expected={n}", file=sys.stderr)
        return 2

    total = 0.0
    maximum = 0
    for x, y in zip(a, b):
        d = abs(x - y)
        total += d
        if d > maximum:
            maximum = d
    mean = total / n
    print(f"max_abs_diff={maximum} mean_abs_diff={mean:.3f} threshold={threshold}")
    return 0 if mean <= threshold else 1


if __name__ == "__main__":
    sys.exit(main())
