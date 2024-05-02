import os, sys
import json, hashlib


def walk_source(path):
    source_paths = []
    subdirs = []

    for dirpath, dirnames, filenames in os.walk(path):
        source_paths += [os.path.join(dirpath, f) for f in filenames if f.endswith(".c")]
        subdirs.append(dirpath)

    return (source_paths, subdirs)


def get_checksums(source_files):
    checksums = {}

    for path in source_files:
        with open(path, "rb") as source_file:
            source = source_file.read()

            checksum = hashlib.md5(source).hexdigest()
            checksums[path] = checksum 

    return checksums


def read_checksum_file():
    checksums = {}

    if not os.path.exists("checksums.txt"):
        return checksums

    with open("checksums.txt", "rb") as checksum_file:
        checksums = json.loads(checksum_file.read())

    return checksums


def write_checksum_file(checksums):
    with open("checksums.txt", "w+") as checksum_file:
        checksum_file.write(json.dumps(checksums))


def get_object_names(path):
    object_names = []

    for file in os.listdir(path):
        if os.path.isfile(os.path.join(path, file)):
            name = os.path.splitext(os.path.basename(file))[0]

            object_names.append(name)

    return object_names


def build(source_path, obj_path, out_path, recompile = False):
    source_paths, subdirs = walk_source(source_path)

    if recompile:
        result = os.system(f"gcc {' '.join(source_paths)} -I {' -I '.join(subdirs)} -o {out_path} -pthread -Wall -std=c17 -pedantic")
        print(result)
        return


    checksums_before = read_checksum_file()
    checksums_now = get_checksums(source_paths)

    object_names = get_object_names(obj_path)
    compile_list = []

    for path in checksums_now:
        name = os.path.splitext(os.path.basename(path))[0]

        if path not in checksums_before or checksums_before[path] != checksums_now[path] or name not in object_names:
            compile_list.append(path)

        if name in object_names:
            object_names.remove(name)

    for object_name in object_names:
        os.remove(f"{obj_path}\\{object_name}.o")

    for path in compile_list:
        name = os.path.splitext(os.path.basename(path))[0]

        os.system(f"gcc -c {path} -I {' -I '.join(subdirs)} -o {obj_path}\\{name}.o -pthread -Wall -std=c17 -pedantic")

    write_checksum_file(checksums_now)
    print(os.system(f"gcc {obj_path}\\*.o -o {out_path} -pthread -Wall -std=c17 -pedantic -g"))


build(sys.argv[1], sys.argv[2], sys.argv[3], True)