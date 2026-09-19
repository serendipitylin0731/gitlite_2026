#!/usr/bin/env python3
"""Synchronize tests and build the native ACMOJ 3226 data package."""

import argparse
import importlib.util
import shutil
import subprocess
import tempfile
import zipfile
from pathlib import Path


ROOT = Path(__file__).resolve().parent.parent
TESTING = ROOT / "testing"
STUDENT = ROOT / "gitlite_stu"
PACKAGE = STUDENT / "3226"


def load_configuration():
    spec = importlib.util.spec_from_file_location("gitlite_tester", TESTING / "tester.py")
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return (module.TEST_SCORES, module.SUBTASKS, module.TEST_DEPENDENCIES,
            module.SUBTASK_CAPS)


def validate(scores, subtasks, dependencies, caps):
    sample_names = {path.stem for path in (TESTING / "samples").glob("*.in")}
    scored_names = set(scores)
    grouped_names = {name for members in subtasks.values() for name in members}
    if sample_names != scored_names or scored_names != grouped_names:
        raise RuntimeError(
            "samples, TEST_SCORES and SUBTASKS differ: "
            f"unscored={sorted(sample_names - scored_names)}, "
            f"missing={sorted(scored_names - sample_names)}, "
            f"ungrouped={sorted(scored_names - grouped_names)}")
    unknown_caps = set(caps) - set(subtasks)
    if unknown_caps:
        raise RuntimeError(f"caps reference unknown subtasks: {sorted(unknown_caps)}")
    for name, required in dependencies.items():
        if name not in scores or any(item not in scores for item in required):
            raise RuntimeError(f"invalid dependency entry for {name}")
    for sample in sorted((TESTING / "samples").glob("*.in")):
        first = next((line for line in sample.read_text(errors="replace").splitlines()
                      if line.strip()), "")
        if not first.lstrip().startswith("#"):
            raise RuntimeError(f"{sample.name} has no leading test-goal comment")
    expected_groups = [10, 10, 20, 20, 25, 35]
    actual_groups = [sum(scores[name] for name in members) for members in subtasks.values()]
    if actual_groups != expected_groups:
        raise RuntimeError(f"subtask totals are {actual_groups}, expected {expected_groups}")
    effective_groups = [min(total, caps.get(name, total))
                        for (name, _), total in zip(subtasks.items(), actual_groups)]
    if effective_groups != [10, 10, 20, 20, 25, 25] or sum(effective_groups) != 110:
        raise RuntimeError(f"effective subtask totals are {effective_groups}, expected 110")
    bonus = subtasks["Subtask6(bonus)"]
    status = sum(scores[name] for name in bonus if "-status-" in name)
    remote = sum(scores[name] for name in bonus
                 if "-remote-" in name or name == "6-robust")
    diff = sum(scores[name] for name in bonus if "-diff-" in name)
    if [status, remote, diff] != [10, 15, 10]:
        raise RuntimeError(
            f"bonus category totals are {[status, remote, diff]}, expected [10, 15, 10]")


def synchronize():
    # The student-facing smoke tests are intentionally smaller and independent
    # from the grading suite.  Rebuilding 3226 must never overwrite them.
    destination = PACKAGE / "samples"
    shutil.rmtree(destination, ignore_errors=True)
    shutil.copytree(TESTING / "samples", destination)
    destination = PACKAGE / "test-assets"
    shutil.rmtree(destination, ignore_errors=True)
    shutil.copytree(TESTING / "src", destination)

    starter = PACKAGE / "starter"
    for name in ["include", "src"]:
        shutil.rmtree(starter / name, ignore_errors=True)
        shutil.copytree(STUDENT / name, starter / name)
    shutil.copy2(STUDENT / "main.cpp", starter / "main.cpp")


def bundle_bytes(total, tests):
    files = sorted(
        [path for directory in [PACKAGE / "samples", PACKAGE / "test-assets"]
         for path in directory.rglob("*") if path.is_file()],
        key=lambda path: path.relative_to(PACKAGE).as_posix())
    output = bytearray()
    output.extend(f"GITLITE_NATIVE_V1\n{total}\n{len(tests)}\n".encode())
    output.extend(("\n".join(tests) + "\n").encode())
    output.extend(f"{len(files)}\n".encode())
    for path in files:
        data = path.read_bytes()
        output.extend(path.relative_to(PACKAGE).as_posix().encode() + b"\n")
        output.extend(str(len(data)).encode() + b"\n")
        output.extend(data + b"\n")
    return bytes(output)


def build_checker():
    command = ["g++", "-std=c++17", "-O2", "-static", "-s",
               str(PACKAGE / "spj.cpp"), "-o", str(PACKAGE / "checker")]
    subprocess.run(command, check=True)


def verify(program, subtasks):
    if program is None:
        return
    program = program.resolve()
    with tempfile.TemporaryDirectory(prefix="gitlite-3226-verify-") as temp:
        temp = Path(temp)
        for index in range(1, 7):
            score, message = temp / f"{index}.score", temp / f"{index}.message"
            subprocess.run([str(PACKAGE / "checker"), str(PACKAGE / f"{index}.in"),
                            str(program), str(PACKAGE / f"{index}.ans"),
                            str(score), str(message)], check=True)
            if score.read_text().strip() != "1.00000000":
                raise RuntimeError(f"group {index} failed:\n{message.read_text()}")


def make_zip():
    archive = STUDENT / "3226.zip"
    temporary = archive.with_suffix(".zip.tmp")
    with zipfile.ZipFile(temporary, "w", zipfile.ZIP_DEFLATED) as output:
        for path in sorted(PACKAGE.rglob("*")):
            if path.is_file():
                output.write(path, (Path("3226") / path.relative_to(PACKAGE)).as_posix())
    temporary.replace(archive)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--program", type=Path,
                        help="compiled complete implementation used to validate all six groups")
    args = parser.parse_args()
    scores, subtasks, dependencies, caps = load_configuration()
    validate(scores, subtasks, dependencies, caps)
    synchronize()
    for index, (group, members) in enumerate(subtasks.items(), 1):
        raw_total = sum(scores[name] for name in members)
        total = min(raw_total, caps.get(group, raw_total))
        (PACKAGE / f"{index}.ans").write_bytes(bundle_bytes(total, members))
        (PACKAGE / f"{index}.in").write_bytes(b"\n")
    build_checker()
    verify(args.program, subtasks)
    make_zip()
    effective_total = sum(
        min(sum(scores[name] for name in members), caps.get(group, float("inf")))
        for group, members in subtasks.items())
    print(f"Built {STUDENT / '3226.zip'}: {len(scores)} tests, "
          f"{effective_total:g} effective points")


if __name__ == "__main__":
    main()
