# C++ Compiler Command Driver (xyo-cc) — Documentation

`xyo-cc` is the compiler driver of the XYO C++ stack. It takes a short,
compiler independent description of a project — sources, include paths,
defines, libraries, resources — and turns it into the right command lines for
**MSVC** (`cl.exe`, `lib.exe`, `link.exe`, `rc.exe`, `cvtres.exe`) or **GCC**
(`gcc` / `g++`, `ar`, `windres`, also MinGW and `emcc` for Emscripten):

- **One description, two tool chains.** The same arguments build with MSVC
  on Windows and with GCC on Linux, MinGW or Emscripten. Paths, separators,
  file extensions, CRT selection, import libraries and system libraries are
  handled for you.
- **Executables, static libraries, DLLs.** `--exe`, `--lib`, `--dll` and
  `--dll-x-static` (a DLL that links its XYO dependencies statically), in
  release or debug, with the dynamic or static C runtime.
- **Incremental and parallel.** Only sources whose object file is missing or
  older than the source or any listed header are compiled, on all
  processors. A change of the compile flags (defines, include paths,
  options, compiler) rebuilds the project; a change of the link command
  relinks it.
- **Arguments from files.** `@file` reads command line arguments from a
  text file, `@file.json` from a JSON project description. `fabricare`
  writes a `.compile.json` for every project and calls `xyo-cc` with it.
- **Also a library.** The compiler classes (`ICompiler`, `CompilerMSVC`,
  `CompilerGCC`) and the command line application are C++ classes you can
  link and call; `fabricare` embeds the whole `xyo-cc` application this way.

```
fabricare                  (build tool, calls xyo-cc for every C/C++ project)
xyo-cc                     <-- this project: application + library
file-json                  (reads @file.json)
xyo-system                 (Shell, File, FileTime, IApplication, ShellArguments)
xyo-encoding               (String, NumberX)
xyo-multithreading         (WorkerQueue: parallel compilation)
xyo-data-structures        (TDynamicArray)
xyo-managed-memory         (Object, TPointer)
xyo-platform               (XYO_PLATFORM_* macros, Processor)
```

## Why use it

- **It is how XYO code is built.** Every `xyo-*`, `quantum-script*` and
  `vendor-*` project with `"type": "xyo-cpp"` is compiled by `fabricare`
  through `xyo-cc`. Knowing its options is knowing what a `fabricare.json`
  project turns into.
- **No build system needed for a small program.** A one-line command, or a
  10-line JSON file, builds an executable or library with incremental,
  parallel compilation, without Makefiles, CMake or a Visual Studio project.
- **Bootstrapping.** `xyo-cc` can build itself and the whole XYO stack from
  the amalgamated sources (`*.Amalgam.cpp`), see
  [`input/`](../input/).
- **Predictable.** The exact commands are printed, every compiler call goes
  through a response file you can inspect in the temp folder, and the
  defines it adds (`XYO_PLATFORM_COMPILE_*`, `<PROJECT>_INTERNAL`, ...) are
  documented here.

## Concepts at a glance

| Need | Use | Notes |
|------|-----|-------|
| Build an executable | `xyo-cc --exe --project=hello --src-cpp=hello.cpp` | output `./hello[.exe]`, objects in `./temp` |
| Build a static library | `--lib` | `<name>.lib` (MSVC) / `<name>.a` (GCC) in `--output-lib-path` |
| Build a DLL / shared library | `--dll` | DLL / `.so` in `--output-bin-path`, import library in `--output-lib-path` |
| DLL with static XYO dependencies | `--dll-x-static` | static CRT, sources compiled as for a static library |
| Debug build | `--debug` | default is `--release` |
| C runtime | `--crt-dynamic` (default) / `--crt-static` | `/MD` / `/MT` with MSVC, static libstdc++ / libgcc with MinGW |
| Sources | `--src-cpp=`, `--src-c=` | C and C++ mixed: C sources are compiled as C++ |
| Headers to watch | `--src-hpp=`, `--src-h=` | a newer header rebuilds every object of the project |
| Defines / include paths | `--def=NAME[=value]`, `--inc=path` | |
| Link libraries | `--use-lib-path=path`, `--use-lib=name` | `:name` = an XYO library by file name, see [linking](build-process.md#linking-libraries) |
| Windows resources | `--rc-src=`, `--rc-inc=`, `--rc-def=` | for executables and DLLs only |
| Output folders | `--output-path=`, `--output-bin-path=`, `--output-lib-path=`, `--temp-path=` | |
| Arguments from a file | `@project.arguments`, `@project.json` | JSON keys: `project`, `type`, `defines`, `includePath`, `cppSource`, ... |
| Choose the tool chain | `--platform-compiler-msvc` / `--platform-compiler-gcc`, `--platform-64bit`, `--platform-os-linux`, ... | default: the tool chain `xyo-cc` itself was built with |
| Rebuild everything | `--force-make` | |
| Call it from C++ | `CompilerMSVC` / `CompilerGCC` -> `makeCppToExe(...)` | or run `Application::main` in process |

## Contents

| Document | What it covers |
|----------|----------------|
| [Getting started](getting-started.md) | Build and install, first builds from the command line and from JSON, use from fabricare, bootstrap without fabricare |
| [Command line](command-line.md) | Every option, `@file` and `@file.json` argument files, environment variables, how the tool chain is selected, exit codes |
| [Build process](build-process.md) | Defines added, incremental rules, object and output file names, exact MSVC / GCC commands, linking libraries, resources, Emscripten |
| [Library](library.md) | Using `ICompiler`, `CompilerMSVC`, `CompilerGCC`, `CompilerOptions`, `CompilerMake` from C++; embedding the application |
| [API reference](reference.md) | Every public symbol and macro on one page |

The types used in the API come from the layers below: `String` from
`xyo-encoding`, `TDynamicArray` from `xyo-data-structures`, `Object` /
`TPointer` / `TMemory` from `xyo-managed-memory`, `Shell` and
`IApplication` from `xyo-system`, `WorkerQueue` from `xyo-multithreading`.
Each repository has its own `docs/`.

## Source map

```
source/XYO/CPPCompilerCommandDriver.hpp                umbrella header of the library
source/XYO/CPPCompilerCommandDriver.Amalgam.cpp        the whole library in one translation unit
source/XYO/CPPCompilerCommandDriver/
    Dependency.hpp                                     xyo-system, export macros
    CompilerType.hpp                                   CompilerType::None / MSVC / GCC
    CompilerOptions[.cpp]                              option bits, filterOptions
    ICompiler.hpp                                      compiler interface, target flags
    CompilerMake[.cpp]                                 shared steps: incremental, parallel compile, resources
    CompilerMSVC[.cpp]                                 cl.exe / lib.exe / link.exe / rc.exe / cvtres.exe
    CompilerGCC[.cpp]                                  gcc / g++ / emcc / ar / windres
    Library.hpp                                        includes all public headers
    Library.rc / .rh                                   version resource of the DLL
    Copyright / License / Version                      library metadata
source/XYO/CPPCompilerCommandDriver.Application.hpp    umbrella header of the application
source/XYO/CPPCompilerCommandDriver.Application.Amalgam.cpp   library + application in one translation unit
source/XYO/CPPCompilerCommandDriver.Application/
    Application[.cpp]                                  command line parsing, @file / @file.json, platform selection
    Application.rc / .ico / .manifest                  Windows resources of xyo-cc.exe
    Copyright / License / Version                      application metadata
input/xyo-cc-x.compile.arguments                       xyo-cc builds itself from amalgam sources, @file form
input/xyo-cc-y.compile.json                            the same with a JSON project description
fabricare/test.js                                      fabricare test: runs both builds above
```

## AI assistant skill

A Claude Code skill describing how to use `xyo-cc` lives in
[`.claude/skills/xyo-cc/`](../.claude/skills/xyo-cc/SKILL.md).
It is picked up automatically inside this repository; copy the folder to
`~/.claude/skills/` to have it available in the projects that build with
`xyo-cc` or link its library.
