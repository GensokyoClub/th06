import argparse
import hashlib
from pathlib import Path
import textwrap
import sys
import os

from configure import BuildType, configure
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


def build(build_type, verbose=False, jobs=1, target=None):
    ninja_args = []
    if verbose:
        ninja_args += ["-v"]

    if jobs != 0:
        ninja_args += ["-j" + str(jobs)]

    if target is not None:
        ninja_args += [target]
    elif build_type == BuildType.TESTS:
        ninja_args += ["build/th06-tests.exe"]
    elif build_type == BuildType.OBJDIFFBUILD:
        ninja_args += ["objdiff"]
    else:
        ninja_args += ["build/th06.exe"]

    configure(build_type)

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
                    "build/th06.exe",
                    "1038721275",  # 2002-12-01 06:41:15
                ]
            )
        diff = find_diff("resources/th06.exe", "build/th06.exe")
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
            print("Exe hash: " + get_sha256("build/th06.exe"), file=sys.stderr)


def main():
    parser = argparse.ArgumentParser(
        "th06-build", formatter_class=argparse.RawTextHelpFormatter
    )
    parser.add_argument(
        "--build-type",
        choices=[
            "normal",
            "diffbuild",
            "tests",
            "objdiffbuild",
            "binary_matchbuild",
            "trial",
        ],
        default="normal",
    )
    parser.add_argument(
        "-j",
        "--jobs",
        type=int,
        default=1,
        help=textwrap.dedent("""
            Number of jobs to run in parallel. Set to 0 to run one job per CPU core. Defaults to 1.
            Note that parallel builds may not work when running through wine.
            See https://github.com/happyhavoc/th06/issues/79 for more information."""),
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
    elif args.build_type == "diffbuild":
        build_type = BuildType.DIFFBUILD
    elif args.build_type == "tests":
        build_type = BuildType.TESTS
    elif args.build_type == "objdiffbuild":
        build_type = BuildType.OBJDIFFBUILD
    elif args.build_type == "binary_matchbuild":
        build_type = BuildType.BINARY_MATCHBUILD
    elif args.build_type == "trial":
        build_type = BuildType.TRIAL

    if args.object_name is not None:
        object_name = Path(args.object_name).name
        target = "build/objdiff/reimpl/" + object_name
    elif args.target is not None:
        target = args.target

    build(build_type, args.verbose, args.jobs, target=target)


if __name__ == "__main__":
    main()
