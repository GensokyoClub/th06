import argparse
import hashlib
from pathlib import Path
import textwrap
import sys
import os

from configure import BuildType, BuildVersion, configure
from winhelpers import run_windows_program

SCRIPTS_DIR = Path(__file__).parent


def get_sha256(path):
    h = hashlib.new("sha256")
    with open(path, "rb") as f:
        while True:
            data = f.read(16 * 4096 * 4096)
            if not data:
                break
            h.update(data)
    return h.hexdigest()


def find_diff(path1, path2):
    offset = 0
    with open(path1, "rb") as file1, open(path2, "rb") as file2:
        while True:
            page1 = file1.read(0x1000)
            if not page1:
                return None
            page2 = file2.read(0x1000)
            if page1 != page2:
                for i, (byte1, byte2) in enumerate(zip(page1, page2)):
                    if byte1 != byte2:
                        return (offset + i, byte1, byte2)
            offset += 0x1000


def build(
    build_type, build_version, ver_suffix, timestamp, verbose=False, jobs=1, target=None
):
    ninja_args = []
    if verbose:
        ninja_args += ["-v"]

    if jobs != 0:
        ninja_args += ["-j" + str(jobs)]

    build_exe = "build/th06" + ver_suffix + ".exe"
    original_exe = "resources/th06" + ver_suffix + ".exe"

    if target is not None:
        ninja_args += [target]
    elif build_type == BuildType.TESTS:
        ninja_args += ["build/th06-tests.exe"]
    elif build_type == BuildType.OBJDIFFBUILD:
        ninja_args += ["objdiff"]
    else:
        ninja_args += [build_exe]

    configure(build_type, build_version, ver_suffix)

    # Use the original MSVC toolchain through the project's Windows environment.
    run_windows_program(
        [str(SCRIPTS_DIR / "th06run.bat"), "ninja"] + ninja_args,
        cwd=str(SCRIPTS_DIR.parent),
    )

    if build_type == BuildType.BINARY_MATCHBUILD:
        if os.path.isfile("build/th06.exe"):
            run_windows_program(
                [
                    sys.executable,
                    str(SCRIPTS_DIR / "patch_timestamp.py"),
                    build_exe,
                    timestamp,
                ]
            )
        diff = find_diff(original_exe, build_exe)
        if diff is None:
            print("Binary matches!", file=sys.stderr)
        else:
            print(
                "Diff at byte "
                + hex(diff[0])
                + ": "
                + hex(diff[1])
                + " "
                + hex(diff[2]),
                file=sys.stderr,
            )
            print("Exe hash: " + get_sha256(build_exe), file=sys.stderr)


def main():
    parser = argparse.ArgumentParser(
        "th06-build", formatter_class=argparse.RawTextHelpFormatter
    )
    parser.add_argument(
        "--build-type",
        choices=["normal", "tests", "objdiffbuild", "binary_matchbuild"],
        default="normal",
    )
    parser.add_argument(
        "--build-version",
        choices=[
            "0.08p",
            "0.13",
            "0.13a",
            "1.00",
            "1.01",
            "1.02",
            "1.02a",
            "1.02b",
            "1.02c",
            "1.02d",
            "1.02e",
            "1.02f",
            "1.02g",
            "1.02h",
        ],
        default="1.02h",
    )
    parser.add_argument(
        "-j",
        "--jobs",
        type=int,
        default=1,
        help=textwrap.dedent("""
            Number of jobs to run in parallel. Set to 0 to run one job per CPU core. Defaults to 1."""),
    )
    parser.add_argument("--verbose", action="store_true")
    parser.add_argument("--object-name", required=False)
    parser.add_argument(
        "target",
        nargs="?",
        help=textwrap.dedent("""
        Ninja target to build. Default depends on the build type:
          - Normal and diff builds will build th06.exe
          - Test builds will build th06-tests.exe
          - objdiff builds will build all the object files necessary for objdiff.
    """),
    )
    args = parser.parse_args()
    target = None

    # First, create the build.ninja file that will be used to build.
    if args.build_type == "normal":
        build_type = BuildType.NORMAL
    elif args.build_type == "tests":
        build_type = BuildType.TESTS
    elif args.build_type == "objdiffbuild":
        build_type = BuildType.OBJDIFFBUILD
    elif args.build_type == "binary_matchbuild":
        build_type = BuildType.BINARY_MATCHBUILD

    if args.build_version == "0.08p":
        build_version = BuildVersion.VER_008p
        ver_suffix = "_008p"
        timestamp = "1024682509"
    elif args.build_version == "0.13":
        build_version = BuildVersion.VER_013
        ver_suffix = "_013"
        timestamp = "1030279914"
    elif args.build_version == "0.13a":
        build_version = BuildVersion.VER_013a
        ver_suffix = "_013a"
        timestamp = "1030719702"
    elif args.build_version == "1.00":
        build_version = BuildVersion.VER_100
        ver_suffix = "_100"
        timestamp = "1028439152"
    elif args.build_version == "1.01":
        build_version = BuildVersion.VER_101
        ver_suffix = "_101"
        timestamp = "1029333979"
    elif args.build_version == "1.02":
        build_version = BuildVersion.VER_102
        ver_suffix = "_102"
        timestamp = "1029858216"
    elif args.build_version == "1.02a":
        print("Version 1.02a has the same binary as 1.02", file=sys.stderr)
        build_version = BuildVersion.VER_102
        ver_suffix = "_102"
        timestamp = "1029858216"
    elif args.build_version == "1.02b":
        build_version = BuildVersion.VER_102b
        ver_suffix = "_102b"
        timestamp = "1029937938"
    elif args.build_version == "1.02c":
        build_version = BuildVersion.VER_102c
        ver_suffix = "_102c"
        timestamp = "1029944083"
    elif args.build_version == "1.02d":
        build_version = BuildVersion.VER_102d
        ver_suffix = "_102d"
        timestamp = "1030258181"
    elif args.build_version == "1.02e":
        print("Version 1.02e has the same binary as 1.02d", file=sys.stderr)
        build_version = BuildVersion.VER_102d
        ver_suffix = "_102d"
        timestamp = "1030258181"
    elif args.build_version == "1.02f":
        build_version = BuildVersion.VER_102f
        ver_suffix = "_102f"
        timestamp = "1030706852"
    elif args.build_version == "1.02g":
        build_version = BuildVersion.VER_102g
        ver_suffix = "_102g"
        timestamp = "1038677883"
    elif args.build_version == "1.02h":
        build_version = BuildVersion.VER_102h
        ver_suffix = ""
        timestamp = "1038721275"  # 2002-12-01 06:41:15

    if args.object_name is not None:
        object_name = Path(args.object_name).name
        target = "build/objdiff/reimpl/" + object_name
    elif args.target is not None:
        target = args.target

    build(
        build_type,
        build_version,
        ver_suffix,
        timestamp,
        args.verbose,
        args.jobs,
        target=target,
    )


if __name__ == "__main__":
    main()
