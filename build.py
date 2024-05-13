import json
import subprocess
from subprocess import PIPE
from hashlib import md5
from pathlib import Path


GCC = "gcc"
ARGS = "-fdiagnostics-color -pthread -Wall -std=c17 -pedantic -g"


SOURCE_DIR = Path("source/")
OBJECT_DIR = Path("objects/")
OUTPUT = Path("build/debug")


SRC_CHECKSUMS_TXT = Path("build/src_checksums.txt")
HDR_CHECKSUMS_TXT = Path("build/hdr_checksums.txt")
ERRORS_TXT = Path("build/errors.txt")


def scan_checksums(files) -> list[str]:
    return [md5(file.read_bytes()).hexdigest() for file in files]


def read_text(txt: Path) -> any:
    if not txt.exists():
        return []
    
    content = txt.read_text("utf-8")
    return json.loads(content) if content else []


def write_text(txt: Path, content) -> None:
    txt.write_text(json.dumps(content))


def checksum_difs(cur, old, files) -> set[str]:
    dif = set(cur) - set(old)
    return ({files[cur.index(i)] for i in dif}, dif)


def delete_unused_objects(object_dir: Path, source_stems: list[str]) -> None:
    objects = [object for object in object_dir.rglob("*.o")]

    map(Path.unlink, [object for object in objects if object.stem not in source_stems])


def paths_to_args(args: list[Path], sep: str="") -> str:
    return " ".join(f"{sep}{arg}" for arg in args)


def dispatch(args):
    return subprocess.Popen(args, stdout=PIPE, stderr=PIPE, text=True, encoding="utf-8")


def header_deps(sources, includes):
    incl = paths_to_args(includes, "-I ")
    src = paths_to_args(sources)
    out, err = dispatch(f"{GCC} -MM {src} {ARGS} {incl}").communicate()

    dependencies = []
    if out:
        for line in out.splitlines():
            if ":" in line:
                dependencies.append(set())

            dependencies[-1].add(Path(line.strip(" \\")))

    return dependencies


def compile_job(sources, includes, errors, object_dir):
    incl = paths_to_args(includes, "-I ")

    processes = []
    for source in sources:
        output = object_dir.joinpath(source.with_suffix(".o").name)

        processes.append(dispatch(f"{GCC} -c {source} {ARGS} {incl} -o {output}"))

    for process, source in zip(processes, sources):
        out, err = process.communicate()
        errors[source.stem] = err


def link_job(object_dir, output):
    out, err = dispatch(f"{GCC} {object_dir}/*.o {ARGS} -o {output}").communicate()
    return err


def build(src: Path, object_dir: Path, output: Path):
    # Walk source directory, obtain checksums
    includes, headers, sources = map(list, map(src.rglob, ["*/", "*.h", "*.c"]))

    hc_cur, sc_cur = scan_checksums(headers), scan_checksums(sources)
    hc_old, sc_old = read_text(HDR_CHECKSUMS_TXT), read_text(SRC_CHECKSUMS_TXT)

    # Find out which sources need to be compiled based on checksum differences
    # and dependencies on changed headers
    header_updates, hc_updates = checksum_difs(hc_cur, hc_old, headers)
    source_updates, sc_updates = checksum_difs(sc_cur, sc_old, sources)

    for source_dependencies, source in zip(header_deps(sources, includes), sources):
        if any(header in header_updates for header in source_dependencies):
            source_updates.add(source)

    # Compile step: Read old error messages, then update, print, and write them
    errors = dict(read_text(ERRORS_TXT))
    compile_job(source_updates, includes, errors, object_dir)

    error_amt = 0
    source_stems = [source.stem for source in sources]
    for source_stem, message in list(errors.items()):
        if source_stem not in source_stems:
            errors.pop(source_stem)
        elif message:
            print(message)
            error_amt += 1
        else:
            sc_updates.remove(sc_cur[source_stems.index(source_stem)])

    write_text(HDR_CHECKSUMS_TXT, list(hc_updates))
    write_text(SRC_CHECKSUMS_TXT, list(sc_updates))
    write_text(ERRORS_TXT, errors)

    # Link step: Delete unused objects, link and print the error message
    delete_unused_objects(object_dir, [source.stem for source in sources])
    link_err = link_job(object_dir, output)

    if link_err:
        print(link_err)

    # yippee
    print(f"Compiled: {len(source_updates)} Linked: {len(sources)} Errored: {error_amt}")


if __name__ == "__main__":
    build(SOURCE_DIR, OBJECT_DIR, OUTPUT)