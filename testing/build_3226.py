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
    return module.TEST_SCORES, module.SUBTASKS, module.TEST_DEPENDENCIES


def validate(scores, subtasks, dependencies):
    sample_names = {path.stem for path in (TESTING / "samples").glob("*.in")}
    scored_names = set(scores)
    grouped_names = {name for members in subtasks.values() for name in members}
    if sample_names != scored_names or scored_names != grouped_names:
        raise RuntimeError(
            "samples, TEST_SCORES and SUBTASKS differ: "
            f"unscored={sorted(sample_names - scored_names)}, "
            f"missing={sorted(scored_names - sample_names)}, "
            f"ungrouped={sorted(scored_names - grouped_names)}")
    if sum(scores.values()) != 110:
        raise RuntimeError(f"test scores total {sum(scores.values())}, expected 110")
    for name, required in dependencies.items():
        if name not in scores or any(item not in scores for item in required):
            raise RuntimeError(f"invalid dependency entry for {name}")
    for sample in sorted((TESTING / "samples").glob("*.in")):
        first = next((line for line in sample.read_text(errors="replace").splitlines()
                      if line.strip()), "")
        if not first.lstrip().startswith("#"):
            raise RuntimeError(f"{sample.name} has no leading test-goal comment")
    expected_groups = [10, 10, 20, 20, 20, 30]
    actual_groups = [sum(scores[name] for name in members) for members in subtasks.values()]
    if actual_groups != expected_groups:
        raise RuntimeError(f"subtask totals are {actual_groups}, expected {expected_groups}")


def synchronize():
    for destination in [STUDENT / "testing" / "samples", PACKAGE / "samples"]:
        shutil.rmtree(destination, ignore_errors=True)
        shutil.copytree(TESTING / "samples", destination)
    for destination in [STUDENT / "testing" / "src", PACKAGE / "test-assets"]:
        shutil.rmtree(destination, ignore_errors=True)
        shutil.copytree(TESTING / "src", destination)
    shutil.copy2(TESTING / "tester.py", STUDENT / "testing" / "tester.py")

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
    scores, subtasks, dependencies = load_configuration()
    validate(scores, subtasks, dependencies)
    synchronize()
    for index, members in enumerate(subtasks.values(), 1):
        total = sum(scores[name] for name in members)
        (PACKAGE / f"{index}.ans").write_bytes(bundle_bytes(total, members))
        (PACKAGE / f"{index}.in").write_bytes(b"\n")
    build_checker()
    verify(args.program, subtasks)
    make_zip()
    print(f"Built {STUDENT / '3226.zip'}: {len(scores)} tests, {sum(scores.values())} points")


if __name__ == "__main__":
    main()
