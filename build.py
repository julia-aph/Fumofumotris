import hashlib
import json
<<<<<<< HEAD
import subprocess
from pathlib import Path


GCC = "gcc"
ARGS = "-fdiagnostics-color -pthread -Wall -std=c17 -pedantic"
=======
import os
import subprocess
import sys


def walk_source_dir(path: str) -> tuple[list[str], list[str]]:
    source_paths : list[str] = []
    subdirs = []

    for dirpath, dirnames, filenames in os.walk(path):
        source_paths += [os.path.join(dirpath, f) for f in filenames if f.endswith(".c")]
        subdirs.append(dirpath)

    return (source_paths, subdirs)
>>>>>>> 41f57d5ba85e72cf801e8ee91afe55d40d535701


SOURCE_DIR = Path("source/")
OBJECT_DIR = Path("objects/")
OUTPUT = Path("debug")


CHECKSUMS = Path("checksums.txt")
ERRORS = Path("errors.txt")


def disk_scan_chksms(sources: list[Path]) -> list[str]:
    chksms: list[str] = []

    for source in sources:
        with open(source, "rb") as file:
            raw = file.read()
            chksms.append(hashlib.md5(raw).hexdigest())

    return chksms


def disk_read_chksms(txt: Path) -> tuple[list[Path], list[str]]:
    sources: list[Path]
    chksms: list[str]

    if not txt.exists():
        return ([], [])

    with open(txt, "rb") as file:
        zipped: dict[str, str] = json.loads(file.read())

        sources, chksms = [Path(key) for key in zipped.keys()], zipped.values()

    return (sources, chksms)


<<<<<<< HEAD
def disk_write_chksms(txt: Path, sources: list[Path], chksms: list[str]) -> None:
    zipped = {str(source): chksm for source, chksm in zip(sources, chksms)}
=======
def build(source_path, obj_path, out_path, recompile):
    source_paths, subdirs = walk_source_dir(source_path)
>>>>>>> 41f57d5ba85e72cf801e8ee91afe55d40d535701

    with open(txt, "w+") as file:
        file.write(json.dumps(zipped))


def filter_chksms(sources, chksms, old_chksms) -> list[Path]:
    difs = set(chksms).difference(old_chksms)
    return [sources[chksms.index(dif)] for dif in difs]


<<<<<<< HEAD
def scan_sources(source_dir: Path) -> tuple[list[Path], list[Path]]:
    sources = [source for source in source_dir.rglob("*.c")]
    chksms = disk_scan_chksms(sources)

    old_sources, old_chksms = disk_read_chksms(CHECKSUMS)
    updated_sources = filter_chksms(sources, chksms, old_chksms)

    disk_write_chksms(CHECKSUMS, sources, chksms)
    return (updated_sources, sources)


def clean_objects(object_dir: Path, sources: list[Path]) -> None:
    objects: list[Path] = [object for object in object_dir.rglob("*.o")]

    object_stems = [object.stem for object in objects]
    source_stems = [source.stem for source in sources]

    for stem in set(object_stems).difference(source_stems):
        objects[object_stems.index(stem)].unlink()


def compile(source: Path, includes: list[Path], object_dir: Path):
    include_arg: str = " ".join(f"-I {dir}" for dir in includes)
    output: Path = object_dir / source.with_suffix(".o").name

    args = f"{GCC} -c {source} {ARGS} {include_arg} -o {output}"

    return subprocess.Popen(args, stderr=subprocess.PIPE)


def disk_read_errors(txt: Path) -> dict[str, str]:
    if not txt.exists():
        return {}

    with open(txt, "rb") as file:
        return json.loads(file.read())
    

def disk_write_errors(txt: Path, errs: dict[str, str]):
    with open(txt, "w+") as file:
        file.write(json.dumps(errs))


def wait_compile_tasks(tasks, updated_sources) -> dict[str, str]:
    errors = disk_read_errors(ERRORS)

    for task, source in zip(tasks, updated_sources):
        out, err = task.communicate()
        if err:
            errors[str(source)] = err.decode("utf-8")
        else:
            errors[str(source)] = False

    disk_write_errors(ERRORS, errors)
    return errors


def link(object_dir: Path, output: Path) -> None:
    subprocess.run(f"{GCC} -g {object_dir}/*.o {ARGS} -o {output}")


def build(source_dir: Path, object_dir: Path, output: Path):
    updated_sources, all_sources = scan_sources(source_dir)
    includes = [path for path in source_dir.rglob("*") if path.is_dir()]


    tasks = []
    for source in updated_sources:
        tasks.append(compile(source, includes, object_dir))

    errors = wait_compile_tasks(tasks, updated_sources).values()

    print("\n".join(err for err in errors if err is not False).strip("\n"))


    clean_objects(object_dir, all_sources)
    link(object_dir, output)
    print(f"Compiled: {len(updated_sources)} Linked: {len(all_sources)}")


if __name__ == "__main__":
    build(SOURCE_DIR, OBJECT_DIR, OUTPUT)
=======
build("source/", "objects/", "debug", True)
>>>>>>> 41f57d5ba85e72cf801e8ee91afe55d40d535701
