#!/usr/bin/env python3

###
# Generates build files for the project.
# This file also includes the project configuration,
# such as compiler flags and the object matching status.
#
# Usage:
#   python3 configure.py
#   ninja
#
# Append --help to see available options.
###

import argparse
import sys
import os
import glob
from pathlib import Path
from typing import Any, Dict, List, Tuple

from tools.project import (
    Object,
    ProgressCategory,
    ProjectConfig,
    calculate_progress,
    generate_build,
    is_windows,
)

# Game versions
DEFAULT_VERSION = 0 # Japan
VERSIONS = [
    "GLMJ01",   # Japan
    "GLME01",   # US
    "GLME01_1", # US Demo
    "GLMP01",   # EU Demo
    "GLMP01_1", # EU Rev 0
    "GLMP01_2", # EU Rev 1
]

parser = argparse.ArgumentParser()
parser.add_argument(
    "mode",
    choices=["configure", "progress"],
    default="configure",
    help="script mode (default: configure)",
    nargs="?",
)
parser.add_argument(
    "-v",
    "--version",
    choices=VERSIONS,
    type=str.upper,
    default=VERSIONS[DEFAULT_VERSION],
    help="version to build",
)
parser.add_argument(
    "--build-dir",
    metavar="DIR",
    type=Path,
    default=Path("build"),
    help="base build directory (default: build)",
)
parser.add_argument(
    "--binutils",
    metavar="BINARY",
    type=Path,
    help="path to binutils (optional)",
)
parser.add_argument(
    "--compilers",
    metavar="DIR",
    type=Path,
    help="path to compilers (optional)",
)
parser.add_argument(
    "--map",
    action="store_true",
    help="generate map file(s)",
)
parser.add_argument(
    "--debug",
    action="store_true",
    help="build with debug info (non-matching)",
)
parser.add_argument(
    "--action",
    action="store_true",
    help="build for github actions (do not use)",
)
if not is_windows():
    parser.add_argument(
        "--wrapper",
        metavar="BINARY",
        type=Path,
        help="path to wibo or wine (optional)",
    )
parser.add_argument(
    "--dtk",
    metavar="BINARY | DIR",
    type=Path,
    help="path to decomp-toolkit binary or source (optional)",
)
parser.add_argument(
    "--objdiff",
    metavar="BINARY | DIR",
    type=Path,
    help="path to objdiff-cli binary or source (optional)",
)
parser.add_argument(
    "--sjiswrap",
    metavar="EXE",
    type=Path,
    help="path to sjiswrap.exe (optional)",
)
parser.add_argument(
    "--verbose",
    action="store_true",
    help="print verbose output",
)
parser.add_argument(
    "--non-matching",
    dest="non_matching",
    action="store_true",
    help="builds equivalent (but non-matching) or modded objects",
)
parser.add_argument(
    "--no-progress",
    dest="progress",
    action="store_false",
    help="disable progress calculation",
)
args = parser.parse_args()

config = ProjectConfig()
config.version = str(args.version)
version_num = VERSIONS.index(config.version)

# Apply arguments
config.build_dir = args.build_dir
config.dtk_path = args.dtk
config.objdiff_path = args.objdiff
config.binutils_path = args.binutils
config.compilers_path = args.compilers
config.generate_map = args.map
config.non_matching = args.non_matching
config.sjiswrap_path = args.sjiswrap
config.progress = args.progress
if not is_windows():
    config.wrapper = args.wrapper
# Don't build asm unless we're --non-matching
if not config.non_matching:
    config.asm_dir = None

# Tool versions
config.binutils_tag = "2.42-1"
config.compilers_tag = "20240706"
config.dtk_tag = "v1.4.1"
config.objdiff_tag = "v2.7.1"
config.sjiswrap_tag = "v1.2.0"
config.wibo_tag = "0.6.16"

# Project

## Helper (Guess who generated this)
def cflags_paths_expand(cflags):
    seen = set()
    converted = []

    for flag in cflags:
        if flag.startswith("-i "):
            path = flag[3:]
            if any(c in path for c in "*?[]"):
                matches = glob.glob(path, recursive=True)
                for p in matches:
                    norm_path = os.path.normpath(p)
                    flag_str = f"-i {norm_path}"
                    if os.path.isdir(p) and flag_str not in seen:
                        converted.append(flag_str)
                        seen.add(flag_str)
            else:
                if flag not in seen:
                    converted.append(flag)
                    seen.add(flag)
        else:
            if flag not in seen:
                converted.append(flag)
                seen.add(flag)

    cflags[:] = converted

## Definitions
config.config_path = Path("config") / config.version / "config.yml"
config.check_sha_path = Path("config") / config.version / "build.sha1"
config.asflags = [
    "-mgekko",
    "-m__PPCGEKKO__",
    "--strip-local-absolute",
    "-i decomp/Project/lib",
    "-i decomp/Project/sources/*/include",
    "-i decomp/JSystem/**/include",
    "-i decomp/SDK/include",
    "-i decomp/CodeWarrior/**/Include",
    "-i decomp/CodeWarrior/**/INCLUDE",
    "-i decomp/CodeWarrior/**/Inc",
    f"-I {config.build_dir}/{config.version}/include",
    f"--defsym version={version_num}",
]
# fill in * , because MWCC not supporting that
cflags_paths_expand(config.asflags)

config.ldflags = [
    "-fp hardware",
    "-nodefaults",
]
if args.debug:
    config.ldflags.append("-g")
if args.map:
    config.ldflags.append("-mapunused")

# Use for any additional files that should cause a re-configure when modified
config.reconfig_deps = []

# Optional numeric ID for decomp.me preset
# Can be overridden in libraries or objects
config.scratch_preset_id = 91

# Base flags, common to most GC/Wii games.
# Generally leave untouched, with overrides added below.
cflags_base = [
    "-nodefaults",
    "-proc gekko",
    "-align powerpc",
    "-enum int",
    "-fp hardware",
    "-Cpp_exceptions off",
    "-W all", # Oll Korrect
    '-pragma "cats off"',
    '-pragma "warn_notinlined off"',
    "-maxerrors 1",
    "-nosyspath",
    "-str reuse",
    "-multibyte",
    "-i decomp/Project/lib",
    "-i decomp/CodeWarrior/**/Include",
    "-i decomp/CodeWarrior/**/INCLUDE",
    "-i decomp/CodeWarrior/**/Inc",
    f"-i {config.build_dir}/{config.version}/include",
    f"-DVERSION_{config.version}",
]

# Debug flags
if args.debug:
    cflags_base.extend(["-sym on", "-DDEBUG=1"])
else:
    cflags_base.append("-DNDEBUG=1")

if args.action:
    cflags_base.append("-DGH_ACTION=1")

# JAudio flags
cflags_jaudio = [
    *cflags_base,
    "-i decomp/JSystem/**/include",
    "-proc 750",
    "-O4,s",
    "-inline off",
    "-common on",
    "-func_align 32",
    "-lang c++",
    "-DNDEBUG=1", 
    "-w off",
    "-use_lmw_stmw on",
    "-str reuse, readonly",
]

# JSystem library flags
cflags_jsys = [
    *cflags_base,
    "-i decomp/SDK/include",
    "-i decomp/JSystem/**/include",
    "-O4,p",
    "-inline auto",
    "-RTTI on",
    "-fp_contract on",
    
]

# OdemuExi2 library flags
cflags_odemu = [
    *cflags_base,
    "-i decomp/SDK/include",

]
# Metrowerks library flags
cflags_cw = [
    *cflags_base,
    "-i decomp/CodeWarrior/**/Include",
    "-O4,p",
    "-use_lmw_stmw on",
    "-str reuse,pool,readonly",
    "-common off",
    "-inline deferred,auto",
    "-char signed",
]
# Metrowerks TRK library flags
cflags_cw_trk = [
    *cflags_base,
    "-O4,p",
    "-pool off",
    "-str readonly",
    "-enum min",
    "-sdatathreshold 0",
]

# Game flags
cflags_game = [
    *cflags_base,
    "-i decomp/SDK/include",
    "-i decomp/Project/sources/*/include",
    "-i decomp/JSystem/**/include",
    "-O4,p",
    "-RTTI on",
    "-inline auto",
    "-fp_contract on",
]

if config.version == "GLMJ01":
    cflags_odemu.extend(["-O3,p", "-use_lmw_stmw off"])
else:
    cflags_odemu.extend(["-O4,p", "-inline all"])

# SDK flags
cflags_sdk = [
    *cflags_base,
    "-DBUILD_REV=37",
    "-DBUILD_DATE=\"\\\"Jul 19 2001\\\"\"",
    "-DBUILD_TIME=\"\\\"05:43:42\\\"\"",
    "-i decomp/SDK/include",
    "-i decomp/CodeWarrior/PowerPC_EABI_Support/MetroTRK",
    "-O4,p",
    "-inline auto",
]

# fill in * , because MWCC not supporting that
cflags_paths_expand(cflags_base)
cflags_paths_expand(cflags_jaudio)
cflags_paths_expand(cflags_jsys)
cflags_paths_expand(cflags_odemu)
cflags_paths_expand(cflags_cw)
cflags_paths_expand(cflags_cw_trk)
cflags_paths_expand(cflags_game)
cflags_paths_expand(cflags_sdk)

config.linker_version = "GC/1.2.5"

# Helper function for SDK libraries
def SDKLib(lib_name: str, files: List[Tuple[bool, str]], conf: Dict[str,str]={"":""}) -> Dict[str, Any]:
    objects = []
    dirname = f"SDK/src/{lib_name}"
    for matching, filename in files:
        filepath = f"{dirname}/{filename}"
        objects.append(Object(matching, filepath))

    __cflags = cflags_sdk + [f"-i decomp/{dirname}"]

    return {
        "lib": lib_name,
        "cflags": __cflags,
        "progress_category": "sdk",
        "src_dir": "decomp",
        "objects": objects,
        **conf
    }

# Helper function for Dolphin libraries
def DolphinLib(lib_name: str, files: List[Tuple[bool, str]], conf:Dict[str,str]={"":""}) -> Dict[str, Any]:
    return SDKLib(f"dolphin/{lib_name}", files)

# Helper function for JSystem libraries
def JSystemLib(lib_name: str, sub_dir: str, files: List[Tuple[bool, str]], cflags: List[str]=cflags_jsys, conf: Dict[str, str]={"":""}) -> Dict[str, Any]:
    objects = []
    dirpath = f"JSystem/{sub_dir}/src"
    for matching, filename in files:
        filepath = f"{dirpath}/{filename}"
        objects.append(Object(matching, filepath))

    __cflags = cflags_jsys + [f"-i decomp/{dirpath}"]

    return {
        "lib": lib_name,
        "cflags": __cflags,
        "progress_category": "jsys",
        "src_dir": "decomp",
        "objects": objects,
        **conf
    }

# Helper function for CodeWarrior runtime libraries
def CWLib(lib_name: str, sub_path: str, files: List[Tuple[bool, str]], conf: Dict[str, str]={"":""}) -> Dict[str, Any]:
    objects = []
    dirpath = f"CodeWarrior/PowerPC_EABI_Support/{sub_path}"
    for matching, filename in files:
        filepath = f"{dirpath}/{filename}"
        objects.append(Object(matching, filepath))

    __cflags = cflags_cw + [f"-i decomp/{dirpath}"]

    return {
        "lib": lib_name,
        "cflags": __cflags,
        "progress_category": "cw",
        "src_dir": f"decomp",
        "objects": objects,
        **conf
    }

# Helper function for Game app libraries
def GameSource(lib_name: str, files: List[Tuple[bool, str]], conf: Dict[str, str]={"":""}) -> Dict[str, Any]:
    objects = []
    dirpath = f"sources/{lib_name}/src"
    for matching, filename in files:
        filepath = f"{dirpath}/{filename}"
        objects.append(Object(matching, filepath))

    __cflags = cflags_game + [f"-i decomp/{dirpath}"]

    return {
        "lib": lib_name,
        "cflags": cflags_game,
        "progress_category": "game",
        "src_dir": f"decomp/Project",
        "objects": objects,
        **conf
    }
def GameMain(file: Tuple[bool, str], conf: Dict[str, str]={"":""}) -> Dict[str, Any]:
    objects = []
    dirpath = "sources"
    filepath = f"{dirpath}/{file}"
    objects.append(Object(matching, filepath))

    __cflags = cflags_game + [f"-i decomp/{dirpath}"]

    return {
        "lib": lib_name,
        "cflags": cflags_game,
        "progress_category": "game",
        "src_dir": f"decomp/Project",
        "objects": objects,
        **conf
    }

Matching = True                   # Object matches and should be linked
NonMatching = False               # Object does not match and should not be linked
Equivalent = config.non_matching  # Object should be linked when configured with --non-matching


# Object is only matching for specific versions
def MatchingFor(*versions):
    return config.version in versions


config.warn_missing_config = True
config.warn_missing_source = False
config.libs = [

    # Game source folders

#    GameMain("main.cpp")
#    GameSource("system", [
#        (NonMatching, "initthread.cpp"),
#        (NonMatching, "memory.cpp"),
#        (NonMatching, "dvd.cpp"),
#    ]),
#    GameSource("Kawano", [
#        (NonMatching, "sequence.cpp"),
#        (NonMatching, "rolling.cpp"),
#    ]),
#    GameSource("Iwamoto", [
#        (NonMatching, ""),
#    ]),
#    GameSource("Sotoike", [
#        (NonMatching, "AITurara.cpp"),
#    ]),
#    GameSource("Kawamoto", [
#        (NonMatching, ""),
#    ]),
#    GameSource("Koga", [
#        (Matching, "CharacterEventObserver.cpp"),
#        (Matching, "IncludeStrategy.cpp"),
#        (NonMatching, "EnStrategy.cpp"),
#        (NonMatching, "EnemyStrategy.cpp"),
#        (NonMatching, "EnemyTypicalStrategy.cpp"),
#    ]),
#    GameSource("Ajioka", [
#        (NonMatching, ""),
#    ]),
#    GameSource("Sato", [
#        (NonMatching, ""),
#    ]),
#    GameSource("Kohno", [
#        (NonMatching, ""),
#    ]),
#    GameSource("unsorted", [
#        (NonMatching, ""),
#    ]),
    # Kept for reference
            ##Object(NonMatching, "Unsorted/MoveObj.cpp"),
            ##Object(NonMatching, "Unsorted/Character.cpp"), (Sotoike/character.cpp?)
            #Object(NonMatching, "Unsorted/assignPlayerRank.cpp"),
            #Object(NonMatching, "Unsorted/getPlayerRank.cpp"),
            #Object(NonMatching, "Unsorted/checkTimeRange.cpp"),
            ##Object(NonMatching, "Unsorted/checkEventTimeBounds.cpp"),
            ##Object(NonMatching, "Unsorted/setAndExecCurrentEvent.cpp"),

    # JSystem Libraries

    JSystemLib("JKernel", "System/JKernel", [
        (NonMatching, "JKRHeap.cpp"),
        (Matching, "JKRDisposer.cpp"),
        (NonMatching, "JKRThread.cpp"),
    ]),

    JSystemLib("JSupport", "System/JSupport", [
        (Matching, "JSUList.cpp"),
    ]),

    JSystemLib("JUtility", "System/JUtility", [
        (NonMatching, "JUTDirectPrint.cpp"),
    ]),

    # SDK

    DolphinLib("base", [
        (Matching, "PPCArch.c"),
    ]),

    DolphinLib("os", [
        (Matching, "OS.c"),
        (Matching, "OSAlarm.c"),
        (Matching, "OSAlloc.c"),
        (Matching, "OSArena.c"),
        (Matching, "OSAudioSystem.c"),
        (Matching, "OSCache.c"),
        (Matching, "OSContext.c"),
        (Matching, "OSError.c"),
        (Matching, "OSExi.c"),
        (Matching, "OSFont.c"),
        (Matching, "OSInterrupt.c"),
        (Matching, "OSLink.c"),
        (Matching, "OSMessage.c"),
        (Matching, "OSMutex.c"),
        (Matching, "OSReboot.c"),
        (Matching, "OSReset.c"),
        (Matching, "OSResetSW.c"),
        (Matching, "OSRtc.c"),
        (Matching, "OSSerial.c"),
        (Matching, "OSSync.c"),
        (Matching, "OSThread.c"),
        (Matching, "OSTime.c"),
        (Matching, "OSUartExi.c"),
        (Matching, "init/__start.c"),
        (Matching, "init/__ppc_eabi_init.cpp"),
    ],),

    DolphinLib("db", [
        (Matching, "db.c"), 
    ]),

    DolphinLib("mtx", [
        (Matching, "mtx.c"),
        (Matching, "mtx44.c"),
        (Matching, "vec.c"),
        (Matching, "mtxvec.c"), 
    ]),

    DolphinLib("dvd", [
        (Matching, "dvdlow.c"),
        (Matching, "dvdfs.c"),
        (Matching, "dvd.c"),
        (Matching, "dvdqueue.c"),
        (Matching, "dvderror.c"),
        (Matching, "emu_level2/fstload.c"),
    ]),

    DolphinLib("vi", [
        (Matching, "vi.c"),
    ]),

    DolphinLib("pad", [
        (Matching, "Padclamp.c"),
        (Matching, "Pad.c"),
    ]),

    DolphinLib("ai", [
        (Matching, "ai.c"),
    ]),

    DolphinLib("dsp", [
        (Matching, "dsp.c"),
    ]),

    DolphinLib("ar", [
        (Matching, "ar.c"),
        (Matching, "arq.c"), 
    ]),

    DolphinLib("card", [
        (Matching, "CARDBios.c"),
        (Matching, "CARDRdwr.c"),
        (Matching, "CARDBlock.c"),
        (Matching, "CARDDir.c"),
        (Matching, "CARDCheck.c"),
        (Matching, "CARDMount.c"),
        (Matching, "CARDFormat.c"),
        (Matching, "CARDOpen.c"),
        (Matching, "CARDCreate.c"),
        (Matching, "CARDRead.c"),
        (Matching, "CARDWrite.c"),
        (Matching, "CARDStat.c"),
    ]),

    DolphinLib("gx", [
        (Matching, "GXInit.c"),
        (Matching, "GXFifo.c"),
        (Matching, "GXAttr.c"),
        (Matching, "GXMisc.c"),
        (Matching, "GXGeometry.c"),
        (Matching, "GXFrameBuf.c"),
        (Matching, "GXLight.c"),
        (Matching, "GXTexture.c"),
        (Matching, "GXBump.c"),
        (Matching, "GXTev.c"),
        (Matching, "GXPixel.c"),
        (Matching, "GXDraw.c"),
        (Matching, "GXStubs.c"),
        (Matching, "GXDisplayList.c"),
        (Matching, "GXTransform.c"),
    ]),

    # CodeWarrior

#    CWLib("Runtime.PPCEABI.H", "Runtime/Src", [
#        (NonMatching, ""),
#    ]),
#    CWLib("MSL_C.PPCEABI.bare.H", "MSL/MSL_C/MSL_Common/Src", [
#        (NonMatching, ""),
#    ]),
#    CWLib("MSL_C.PPCEABI.bare.H", "MSL/MSL_C/MSL_Common_Embedded/Src", [
#        (NonMatching, ""),
#    ]),
#    CWLib("MSL_C.PPCEABI.bare.H", "MSL/MSL_C/MSL_Common_Embedded/Math", [
#        (NonMatching, ""),
#    ]),
#    CWLib("MSL_C.PPCEABI.bare.H", "MSL/MSL_C/PPC_EABI/SRC", [
#        (NonMatching, ""),
#    ]),
#    CWLib("MSL_C.PPCEABI.bare.H", "MetroTRK/src", [
#        (NonMatching, ""),
#    ],{
#        "cflags": cflags_cw_trk
#    }),
    DolphinLib("amcstubs", [
        (Matching, "AmcExi2Stubs.c"),
    ]),
    SDKLib("OdemuExi2", [
        (Matching, "DebuggerDriver.c")
    ],{
        "cflags": cflags_odemu
    }),
    DolphinLib("odenotstub", [
        (Matching, "odenotstub.c"),
    ]),
]

# Optional extra categories for progress tracking
# Adjust as desired for your project
config.progress_categories = [
    ProgressCategory("game", "Game App"),
    ProgressCategory("jsys", "JSystem"),
    ProgressCategory("sdk", "Dolphin SDK (2001)"),
    ProgressCategory("cw", "CodeWarrior Runtime"),
]
config.progress_each_module = args.verbose
if args.mode == "configure":
    # Write build.ninja and objdiff.json
    generate_build(config)
elif args.mode == "progress":
    # Print progress and write progress.json
    calculate_progress(config)
else:
    sys.exit("Unknown mode: " + args.mode)
