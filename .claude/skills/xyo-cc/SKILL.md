---
name: xyo-cc
description: >-
  How to use xyo-cc, the C++ Compiler Command Driver of the XYO C++ stack
  (namespace XYO::CPPCompilerCommandDriver): the command line tool that turns
  one compiler independent project description into MSVC (cl.exe, lib.exe,
  link.exe, rc.exe, cvtres.exe) or GCC / MinGW / Emscripten (gcc, emcc, ar,
  windres) commands, with incremental parallel builds; its options (--exe,
  --lib, --dll, --dll-x-static, --project, --debug / --release,
  --crt-static / --crt-dynamic, --def, --inc, --src-cpp / --src-c /
  --src-hpp / --src-h, --use-lib-path, --use-lib (":name" XYO libraries),
  --rc-src / --rc-inc / --rc-def, --output-path / --output-bin-path /
  --output-lib-path / --temp-path, --lib-name, --lib-version, --no-lib,
  --force-make, --threads, --platform-compiler-msvc / -gcc,
  --platform-64bit / -32bit, --platform-os-windows / -linux / -emscripten);
  @file argument files and @file.json project descriptions (project, type,
  defines, includePath, cppSource, cSource, hppSource, hSource, crt,
  libraryPath, library, linkerDefinitionsFile, resources); the defines it
  adds (<PROJECT>_INTERNAL, <PROJECT>_DLL_INTERNAL / _LIB_INTERNAL /
  _EXE_INTERNAL, XYO_PLATFORM_COMPILE_RELEASE / DEBUG / CRT_STATIC /
  CRT_DYNAMIC / STATIC_LIBRARY / DYNAMIC_LIBRARY); environment variables
  CC, CXX, XYO_PLATFORM_COMPILE_DEBUG / CRT_STATIC / DEFINE / STATIC; output
  and temp file names (*.cpp2obj, *.obj2exe, *.o2so, *.flags); and the C++
  library: ICompiler, CompilerMSVC, CompilerGCC, CompilerOptions,
  filterOptions, CompilerType, CompilerMake::compileSources,
  makeCppToExe / makeCppToLib / makeCToExe / makeCToLib, and embedding
  Application::main (xyo-cc.application.static, fabricare xyoCC). Use when
  running or writing xyo-cc command lines, .arguments or .compile.json files,
  reading temp/*.compile.json or response files of a fabricare build,
  debugging why an XYO project does (not) rebuild, links the wrong library or
  gets the wrong export macro, calling xyoCC from fabricare scripts,
  writing code that includes <XYO/CPPCompilerCommandDriver.hpp> or depends on
  "xyo-cc" / "xyo-cc.static" / "xyo-cc.application.static" in
  fabricare.json, or when working inside the xyo-cc repository.
---

# xyo-cc

Compiler driver of the XYO C++ stack, on top of `xyo-system` and `file-json`
(see the `xyo-system`, `xyo-managed-memory` and `fabricare` skills; their
rules apply). Purpose: **describe a C/C++ project once, build it with MSVC
or GCC**, incrementally and in parallel. `fabricare` builds every
`"type": "xyo-cpp"` project by writing `temp/<project>.compile.json` and
calling its embedded `xyo-cc` with it.

Full documentation: `docs/` in the xyo-cc repository
(`X:\Storage\XYO\Gitea\CPP\xyo-cc\docs` on this machine): README,
getting-started, **command-line** (every option, JSON keys, environment,
tool chain selection), **build-process** (defines, incremental rules, file
names, exact compiler commands, linking), library, reference. Read the
matching page when you need more than this summary. When in doubt read
`source/XYO/CPPCompilerCommandDriver.Application/Application.cpp` (options)
or `source/XYO/CPPCompilerCommandDriver/Compiler*.cpp` (commands).

## Command line

```bash
xyo-cc --exe --project=hello --inc=source --src-hpp=source/hello.hpp --src-cpp=source/hello.cpp --output-bin-path=output/bin
xyo-cc --lib --project=greet --crt-static --src-cpp=source/greet.cpp --output-lib-path=output/lib
xyo-cc --dll --project=greet --src-cpp=source/greet.cpp --output-bin-path=output/bin --output-lib-path=output/lib
xyo-cc @project.json --debug --output-bin-path=output/bin
```

| Group | Options |
|-------|---------|
| target (one required) | `--exe`, `--lib`, `--dll`, `--dll-x-static` (DLL, static CRT + static XYO deps); several allowed: built lib, dll, exe |
| build | `--project=name` (default `project`), `--release` (default) / `--debug`, `--crt-dynamic` (default) / `--crt-static`, `--threads=n` (default CPUs), `--force-make` |
| sources | `--src-cpp=`, `--src-c=` (compiled), `--src-hpp=`, `--src-h=` (not compiled: a newer header rebuilds **all** objects), `--source-path=` (prefix, default `.`) |
| compiler | `--def=NAME[=value]`, `--inc=path` |
| linker | `--use-lib-path=`, `--use-lib=name` / `:name`, `--def-file=` (MSVC DLL), `--lib-name=` (static lib name), `--lib-version=` (`name-ver.dll`, `name.so.ver`), `--no-lib` (DLL import lib to temp) |
| resources | `--rc-src=`, `--rc-inc=`, `--rc-def=` (exe / dll on Windows only) |
| folders | `--output-path=` (sets bin + lib), `--output-bin-path=`, `--output-lib-path=` (default `.`), `--temp-path=` (default `./temp`) |
| tool chain | `--platform-compiler-msvc` / `-gcc`, `--platform-64bit` / `-32bit`, `--platform-os-windows` / `-linux` / `-emscripten`, `--platform=name` |
| info | `--help`, `--usage`, `--version`, `--license` |

Rules that bite:

- Values attach with `=` only. An unknown option (`--src=`, `--include=`)
  stops with `Error: unknown option - ...`, a bare word (the `source` of
  `--inc source`, a nested `@file`) with `Error: unknown argument - ...`,
  exit 1. The JSON `type` value becomes `--<type>`, so a bad type is an
  unknown option too.
- Single-value options: last wins. List options accumulate. `--output-path`
  resets bin and lib path, so put it first.
- Every `--src-*` file must exist, else `Error: file not found`.
- C and C++ sources together: C is compiled as C++.
- `--crt-dynamic` is the default for every target, `--lib` included (env
  `XYO_PLATFORM_COMPILE_CRT_STATIC=1` changes the default).
- Default tool chain = the one `xyo-cc` was built with. Env
  `XYO_PLATFORM_COMPILER_*` / `_64BIT` / `_OS_*` can only add (MSVC > GCC,
  64 > 32, Windows > Linux/Emscripten); to switch an MSVC-built xyo-cc to
  GCC or Emscripten use `--platform-*` options. MSVC needs a vcvars
  environment (cl.exe on PATH). GCC defaults to `gcc` for C, C++ and
  linking (`-lstdc++` added); `CC` / `CXX` override (`emcc` for Emscripten).
- Exit 0 = ok (also help / no args), 1 = error (`Error: ...` on stdout).

## Argument files

- `@file` (not `.json`): shell-split arguments (spaces / new lines, `"..."`
  groups, `\"` escapes, no comments). One level deep: no nested `@`.
- `@file.json`: object; unknown keys ignored, wrong types are errors.

```json
{
	"project": "tool",
	"type": "exe",
	"defines": ["TOOL_X"],
	"includePath": ["source"],
	"hppSource": [], "hSource": [],
	"cppSource": ["source/XYO/Tool.Amalgam.cpp"],
	"cSource": [],
	"libraryPath": ["output/lib"],
	"library": [":xyo-system", "user32"],
	"linkerDefinitionsFile": "source/tool.def",
	"resources": {"defines": [], "includePath": ["source"], "rcSource": ["source/XYO/Tool/Application.rc"]},
	"crt": "static"
}
```

`type`: `exe` / `lib` / `dll` / `dll-x-static`; `crt`: `static` /
`dynamic`. No JSON keys for output paths, debug, threads, platform: put
them on the command line next to `@file.json`. `input/` has both forms
(xyo-cc building itself from `*.Amalgam.cpp`).

## Defines it adds

| Define | When |
|--------|------|
| `<PROJECT>_INTERNAL` | always; upper case, `-` `.` -> `_`, leading `lib` dropped (`xyo-cc` -> `XYO_CC_INTERNAL`) |
| `<PROJECT>_LIB_INTERNAL` / `_DLL_INTERNAL` / `_EXE_INTERNAL` | `--lib` / `--dll` or `--dll-x-static` / `--exe`; only one, exe > dll > lib if several |
| `XYO_PLATFORM_COMPILE_RELEASE` / `_DEBUG` | build kind |
| `XYO_PLATFORM_COMPILE_CRT_DYNAMIC` / `_CRT_STATIC` | CRT |
| `XYO_PLATFORM_COMPILE_STATIC_LIBRARY` | `--lib`, `--dll-x-static`, `--exe` with env `XYO_PLATFORM_COMPILE_STATIC` |
| `XYO_PLATFORM_COMPILE_DYNAMIC_LIBRARY` | `--dll`, `--exe` (default: exe uses XYO DLLs) |
| `XYO_PLATFORM_COMPILE_STATIC` | env `XYO_PLATFORM_COMPILE_STATIC=1/ON/TRUE` |
| from env | `XYO_PLATFORM_COMPILE_DEFINE` (space separated) |

XYO export macros test `<PREFIX>_DLL_INTERNAL`; if the code prefix differs
from the project name, map it in `Dependency.hpp` (xyo-cc maps `XYO_CC_*`
-> `XYO_CPPCOMPILERCOMMANDDRIVER_*`). Wrong dllimport / dllexport usually
means the project name does not match the prefix.

## Incremental build and files

Compiled if: object missing, older than its source, older than **any**
listed header, saved flags changed (defines, includes, options, target,
`CC`/`CXX` value), or `--force-make`. `#include`s are not scanned. Relinked
if an object is newer than the output or the link response file content
changed. A failed tool call deletes its response file (retry next run).

`<tag>` = R/D (release/debug) + S/D (CRT) + S/D (static lib / dll-or-exe):
`RSS`, `RDD`. `<ext>`: `.lib .dll .exe` (MSVC), `.a .so` / none (GCC).

| File | MSVC | GCC |
|------|------|-----|
| object | `<temp>/<project><ext>.<k>.<tag>.<src>.obj` | `....o` |
| compile response | `<object minus last dot>2obj` e.g. `hello.cpp2obj` | `hello.cpp2o` |
| flags | `<temp>/<project><ext>.<tag>.cpp.flags` (`.c`, `.rc`) | same |
| static lib | `<lib>/<name>.lib` (`<temp>/<name>.obj2lib`) | `<lib>/<name>.a` (`.o2a`), **no `lib` prefix** |
| dll / so | `<bin>/<name>[-ver].dll` + `<lib>/<name>.lib` (`.obj2dll`) | `<bin>/<name>.so[.ver]` or MinGW `.dll`, copied to `<lib>` (`.o2so`) |
| exe | `<bin>/<name>.exe` (`.obj2exe`) | `<bin>/<name>[.exe]` (`.o2elf`) |

To see the exact command of a build, read the response file in temp.

## Linking

| `--use-lib=` | MSVC | GCC |
|--------------|------|-----|
| `name` | `name.lib` | `-lname` (leading `lib` removed) |
| `:name` (XYO lib) | `name.lib` | `-l:name.so` / `-l:name.dll` (MinGW) / `-l:name.a` if `XYO_PLATFORM_COMPILE_STATIC` |
| `:name.static` | `name.static.lib` | `-l:name.static.a` |

MSVC always adds `user32 gdi32 ws2_32 ole32 wininet advapi32 oleaut32
shell32 iphlpapi`; GCC adds `-lstdc++ -lpthread -lm` + `-ldl` (Linux) /
`-luser32 -lws2_32` (Windows) and `-Wl,-rpath='$ORIGIN'`. GCC: order
libraries after their users.

## From fabricare

Scripts: `exitIf(xyoCC("--exe", "--project=x", "--src-cpp=x.cpp"));`
(returns the exit code). `fabricare make` calls
`xyoCC("@temp/<project>.compile.json", "--exe|--lib|--dll", "--output-...")`
plus `--inc` / `--use-lib-path` / `--rc-inc` for `output/`, the SDK and
`../`, and `--debug`. Rerun a project's build by hand with the same
`@temp/<project>.compile.json`.

## C++ library

```cpp
#include <XYO/CPPCompilerCommandDriver.hpp>
using namespace XYO::CPPCompilerCommandDriver;  // also XYO::System

TPointer<ICompiler> compiler;
compiler = TMemory<CompilerMSVC>::newMemory(); // or CompilerGCC
compiler->isOSWindows = true;  // constructor sets ALL flags false: set OS and bits
compiler->is64Bit = true;
TDynamicArray<String> def, inc, hdr, src, rcDef, rcInc, rc, libPath, lib;
inc.push("source");
src.push("source/hello.cpp");
bool ok = compiler->makeCppToExe("hello", "output/bin", "temp",
    CompilerOptions::Release | CompilerOptions::CRTDynamic | CompilerOptions::DynamicLibrary,
    def, inc, hdr, src, rcDef, rcInc, rc, libPath, lib,
    Processor::getCount(), true /*echoCmd*/, false /*force*/);
// makeCppToLib(libName, binPath, libPath, tmpPath, options, def, inc, hdr, src,
//              rcDef, rcInc, rc, defFile, libPath, lib, version, threads, echo, force)
// StaticLibrary bit -> archive, DynamicLibrary bit -> DLL / .so
// makeCToExe / makeCToLib: same for C. Steps: cppToObj, cToObj (different
// parameter order!), makeObjToLib, makeObjToExe, makeRcToObj, objFilename.
```

- `options` is normalized by `filterOptions`: missing bits default to
  Release, CRTDynamic, DynamicLibrary; `DynamicLibraryXStatic` forces
  DynamicLibrary + CRTStatic.
- The library does **not** add `<PROJECT>_*INTERNAL`, read the
  `XYO_PLATFORM_*` env variables, check files exist, or merge C into C++;
  the application does. It does read `CC` / `CXX`.
- `CompilerMake::compileSources(...)` + your objects + `makeObjToExe` /
  `makeObjToLib` = custom pipeline.
- Dependencies: `xyo-cc` (DLL), `xyo-cc.static`, or
  `xyo-cc.application.static` to run the tool in process:
  `XYO::CPPCompilerCommandDriver::Application::Application app;
  int r = app.main(argc, argv);` (`argv[0]` ignored, 0 = ok). Build `argv`
  with `new char *[]`, not a `TDynamicArray` (not contiguous). Call
  `Application::initMemory()` from your `initMemory()` if `String` /
  `TDynamicArray<String>` pools are not set up yet.

## In this repository

- `fabricare make` / `test` (builds `input/xyo-cc-x.compile.arguments` and
  `input/xyo-cc-y.compile.json` into `output/test`) / `install`.
- Options are parsed in `Application.cpp` (also update `showUsage`, the
  `jsonOptions` table and `docs/command-line.md` together); commands are
  built in `CompilerMSVC.cpp` / `CompilerGCC.cpp`; shared incremental logic
  in `CompilerMake.cpp`. Keep MSVC and GCC behavior in step and keep the
  docs in `docs/` current with the code.
