import json
import subprocess
from subprocess import PIPE
from hashlib import md5
from pathlib import Path


GCC = "gcc"
ARGS = "-fdiagnostics-color -pthread -Wall -std=c17 -pedantic -g"


SOURCE_DIR = Path("source/")
OBJECT_DIR = Path("objects/")
OUTPUT = Path("debug")


CHECKSUMS = Path("checksums.txt")
ERRORS = Path("errors.txt")


def scan_checksums(files: list[Path]) -> list[str]:
    return (md5(file.read_bytes()).hexdigest() for file in files)


def read_txt(txt: Path) -> any:
    if not txt.exists():
        return []
    
    content = txt.read_text("utf-8")
    return json.loads(content) if content else []


def write_txt(txt: Path, content) -> None:
    txt.write_text(json.dumps(content))


def difference(cur, old, vals):
    return (vals[cur.index(i)] for i in set(cur) - set(old))


def clean_objects(object_dir: Path, sources: list[Path]) -> None:
    objects: list[Path] = [object for object in object_dir.rglob("*.o")]

    object_stems = [object.stem for object in objects]
    source_stems = [source.stem for source in sources]

    for stem in set(object_stems).difference(source_stems):
        objects[object_stems.index(stem)].unlink()


def header_dependencies(sources):
    tasks = []
    for source in sources:
        task = subprocess.Popen(f"{GCC} -MMD {source} {ARGS} -o stdout", stdout=PIPE)
        tasks.append(task)

    for task in tasks:
        


def compile(source: Path, includes: list[Path], object_dir: Path):
    include_arg: str = " ".join(f"-I {dir}" for dir in includes)
    output: Path = object_dir / source.with_suffix(".o").name

    args = f"{GCC} -c {source} {ARGS} {include_arg} -o {output}"

    return subprocess.Popen(args, stderr=subprocess.PIPE)


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
    subprocess.run(f"{GCC} {object_dir}/*.o {ARGS} -o {output}")


def build(src: Path, object_dir: Path, output: Path):
    includes, headers, sources = zip(map(src.rglob, ["*/", "*.h", "*.c"]))

    headers_cur, sources_cur = map(scan_checksums, (headers, sources))
    headers_old, sources_old = read_txt(CHECKSUMS)

    headers_updt = difference(headers_cur, headers_old, headers)
    sources_updt = difference(sources_cur, sources_old, sources)

    dependencies = {}

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