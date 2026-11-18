#!/usr/bin/env python3
"""compile_commands.json from mkn dry runs, for clang-tidy and clangd: mkn build -RD writes
each compile command, without running it, to .mkn/log/<profile>/obj/cmd/ of the project it
belongs to; this project's are one entry per source, with their real flags (dependencies'
sources are not ours to check). MPI compiler wrappers (mpicxx ...) are replaced by the compiler
and flags they add, which clang-tidy cannot otherwise know.

    python3 res/ci/compile_db.py <profile[,profile...]> [extra mkn args, e.g. -x res/mkn/mpi]
"""

import json
import shlex
import shutil
import subprocess
import sys
from pathlib import Path

LAUNCHERS = {"ccache", "sccache"}
MPI_WRAPPERS = {"mpicc", "mpicxx", "mpic++", "mpiCC"}


def unwrapped(args):
    """args with an MPI wrapper as compiler: the compiler it runs and its flags instead"""
    wrapper = args[0]
    if Path(wrapper).name not in MPI_WRAPPERS:
        return args

    def show(what):
        out = subprocess.run(
            [wrapper, f"--showme:{what}"], capture_output=True, text=True
        )
        return shlex.split(out.stdout) if out.returncode == 0 else []

    compiler = show("command") or [wrapper]
    return compiler[:1] + show("compile") + args[1:]


def main(profiles, extra):
    shutil.rmtree(".mkn/log", ignore_errors=True)  # only this run's commands
    for profile in profiles.split(","):
        subprocess.run(
            ["mkn", "build", "-p", profile, "-RD", *extra],
            check=True,
            stdout=subprocess.DEVNULL,
        )
    cwd = str(Path.cwd())
    entries = {}
    for txt in sorted(Path(".mkn/log").glob("*/*/cmd/*.txt")):
        args = shlex.split(txt.read_text().strip())
        if "-c" not in args:
            continue
        while args and Path(args[0]).name in LAUNCHERS:
            args = args[1:]
        args = unwrapped(args)
        source = args[args.index("-c") + 1]
        entries[source] = {"directory": cwd, "arguments": args, "file": source}
    Path("compile_commands.json").write_text(
        json.dumps(list(entries.values()), indent=1)
    )
    print(f"compile_commands.json: {len(entries)} sources", file=sys.stderr)


if __name__ == "__main__":
    main(sys.argv[1], sys.argv[2:])
