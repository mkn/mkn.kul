#!/usr/bin/env python3
"""clang-tidy's warnings as GitHub annotations (inline on the PR's changed lines), one per
finding, paths relative to the checkout

    python3 res/ci/tidy_annotations.py <clang-tidy log> <checkout root>
"""

import re
import sys
from pathlib import Path

FINDING = re.compile(
    r"^(?P<file>[^:\s]+):(?P<line>\d+):(?P<col>\d+): (?P<kind>warning|error): (?P<msg>.*) \[(?P<check>[^\]]+)\]$"
)


def main(log, root):
    root = Path(root).resolve()
    seen = set()
    for raw in Path(log).read_text(errors="replace").splitlines():
        m = FINDING.match(raw)
        if not m:
            continue
        path = Path(m["file"]).resolve()
        try:
            path = path.relative_to(root)
        except ValueError:
            continue  # not ours (a dependency's header)
        key = (str(path), m["line"], m["col"], m["check"])
        if key in seen:
            continue
        seen.add(key)
        level = "error" if m["kind"] == "error" else "warning"
        print(
            f"::{level} file={path},line={m['line']},col={m['col']},title={m['check']}::{m['msg']}"
        )
    print(f"{len(seen)} clang-tidy finding(s)", file=sys.stderr)


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2])
