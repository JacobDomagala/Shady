#!/usr/bin/env python3
"""Run Cppcheck and Clang-Tidy on CMake's compilation database, retaining full logs."""

import argparse
from concurrent.futures import ThreadPoolExecutor
import json
import os
from pathlib import Path
import shutil
import subprocess


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("build_dir", type=Path)
    parser.add_argument("--clang-tidy", default="clang-tidy")
    parser.add_argument("--cppcheck", default="cppcheck")
    parser.add_argument("--jobs", type=int, default=min(4, os.cpu_count() or 1))
    args = parser.parse_args()
    if args.jobs < 1:
        parser.error("--jobs must be positive")
    for executable in (args.clang_tidy, args.cppcheck):
        if not shutil.which(executable):
            parser.error(f"Executable not found: {executable}")

    root = Path(__file__).resolve().parents[1]
    build_dir = args.build_dir.resolve()
    database = build_dir / "compile_commands.json"
    if not database.is_file():
        parser.error("Configure CMake with -DCMAKE_EXPORT_COMPILE_COMMANDS=ON first")
    entries = json.loads(database.read_text(encoding="utf-8"))
    sources = set()
    for entry in entries:
        source = (Path(entry["directory"]) / entry["file"]).resolve()
        if source.is_relative_to(root / "src") and source.suffix in (".c", ".cc", ".cpp", ".cxx"):
            sources.add(source)
    if not sources:
        parser.error("The compilation database contains no engine source files")

    logs = build_dir / "code-quality"
    logs.mkdir(parents=True, exist_ok=True)

    def run(command, log_path):
        with log_path.open("w", encoding="utf-8") as log:
            result = subprocess.run(command, cwd=root, stdout=log, stderr=subprocess.STDOUT)
        return result.returncode

    for name, executable in (("clang-tidy", args.clang_tidy), ("cppcheck", args.cppcheck)):
        version_log = logs / f"{name}-version.txt"
        if run([executable, "--version"], version_log):
            parser.error(f"Could not run {executable}; see {version_log}")
        print(version_log.read_text(encoding="utf-8", errors="replace"), flush=True)

    # Headers are analyzed through their translation units. Passing them to
    # --file-filter individually fails because they have no compile commands.
    cppcheck_log = logs / "cppcheck.txt"
    cppcheck_result = run(
        [args.cppcheck, f"--project={database}", "--file-filter=*/src/*",
         "--enable=warning,style,performance,portability", "--error-exitcode=1",
         "--suppress=functionStatic", "--suppress=functionConst",
         "--suppress=constParameterPointer", "--suppress=returnByReference",
         "--suppress=funcArgNamesDifferent"],
        cppcheck_log,
    )
    print(cppcheck_log.read_text(encoding="utf-8", errors="replace"), flush=True)

    def tidy(source):
        relative = source.relative_to(root)
        log_path = logs / ("clang-tidy-" + "_".join(relative.parts) + ".txt")
        result = run(
            [args.clang_tidy, "-p", str(build_dir), "--warnings-as-errors=*",
             "--extra-arg=-DFMT_USE_CONSTEVAL=0", str(source)],
            log_path,
        )
        return relative, result, log_path

    failed = cppcheck_result != 0
    with ThreadPoolExecutor(max_workers=args.jobs) as pool:
        for source, result, log_path in pool.map(tidy, sorted(sources)):
            print(f"Clang-Tidy {source}: {'FAILED' if result else 'passed'}", flush=True)
            if result:
                print(log_path.read_text(encoding="utf-8", errors="replace"), flush=True)
                failed = True
    print(f"Complete analysis logs: {logs}", flush=True)
    return int(failed)


if __name__ == "__main__":
    raise SystemExit(main())
