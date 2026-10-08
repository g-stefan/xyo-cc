# Command line

```
xyo-cc [--option[=value] | @file | @file.json] ...
```

All options start with `--`; a value is attached with `=` (`--inc=source`,
never `--inc source`). Running `xyo-cc` without arguments prints the usage.

An unknown option stops with `Error: unknown option - --src=main.cpp` and
an argument that does not start with `--` or `@` with
`Error: unknown argument - source` (as in the mistaken `--inc source`),
exit code 1, before anything is built.

Options are processed left to right. Options with one value (`--project`,
`--temp-path`, `--crt-*`, `--debug` / `--release`, `--platform-*`, ...) keep
the **last** value given; list options (`--def`, `--inc`, `--src-*`,
`--use-lib*`, `--rc-*`) **accumulate**.

## Information

| Option | Effect |
|--------|--------|
| `--help`, `--usage` | print the usage, exit 0 |
| `--license` | print the license, exit 0 |
| `--version` | print `version X.Y.Z build N [date time]`, exit 0 |

These act as soon as they are reached, before anything is built.

## What to build

| Option | Effect |
|--------|--------|
| `--project=name` | project name (default `project`). Names the outputs and the `<NAME>_INTERNAL` defines |
| `--exe` | build an executable |
| `--lib` | build a static library |
| `--dll` | build a DLL / shared library |
| `--dll-x-static` | build a DLL that links its XYO dependencies statically: forces the static CRT and compiles with `XYO_PLATFORM_COMPILE_STATIC_LIBRARY` |
| `--release` | optimized build (default) |
| `--debug` | debug build: no optimization, debug info |
| `--crt-dynamic` | dynamic C runtime (default): `/MD` / `/MDd` with MSVC |
| `--crt-static` | static C runtime: `/MT` / `/MTd` with MSVC, `-static-libstdc++ -static-libgcc` for MinGW executables |
| `--threads=count` | parallel compilations, default the number of processors (also when `count` is not a number) |
| `--force-make` | rebuild everything, ignore file times and saved flags |

At least one of `--exe`, `--lib`, `--dll`, `--dll-x-static` is required,
otherwise: `Error: no exe/dll/lib specified to compile`. More than one may
be given; they are built in the order library, DLL, executable, from the
same sources.

## Sources

| Option | Effect |
|--------|--------|
| `--src-cpp=file` | C++ source to compile |
| `--src-c=file` | C source to compile |
| `--src-hpp=file` | C++ header: not compiled, a dependency of every source |
| `--src-h=file` | C header: not compiled, a dependency of every source |
| `--source-path=path` | folder the `--src-*` and `--rc-src` files are relative to, default `.` |

Every listed file must exist (`<source-path>/<file>`), otherwise
`Error: file not found <file>` and exit 1. If both C and C++ sources are
given, the C sources are compiled as C++ (`Info: C and C++ sources mixed,
C sources are compiled as C++`). Headers are optional; list the ones that
should trigger a rebuild when they change.

## Compiler

| Option | Effect |
|--------|--------|
| `--def=NAME` / `--def=NAME=value` | preprocessor define for C / C++ sources |
| `--inc=path` | include path for C / C++ sources |

## Linker

| Option | Effect |
|--------|--------|
| `--use-lib-path=path` | library search path |
| `--use-lib=name` | link a library: `name` -> `name.lib` (MSVC), `-lname` (GCC, a leading `lib` is removed). `:name` links an XYO library by its file name, see [linking libraries](build-process.md#linking-libraries) |
| `--def-file=file` | module definition file for an MSVC DLL (`/DEF:`) |
| `--lib-name=name` | output name of a static library (`--lib`), default the project name |
| `--lib-version=version` | versioned DLL / shared library: `name-version.dll`, `name.so.version` (with `-soname`) |
| `--no-lib` | with `--dll`: write the import library to the temp folder instead of `--output-lib-path` |

## Windows resources

| Option | Effect |
|--------|--------|
| `--rc-src=file` | resource script (`.rc`) to compile and link |
| `--rc-inc=path` | include path for the resource compiler |
| `--rc-def=NAME[=value]` | define for the resource compiler |

Resources are linked into executables and DLLs, never into static
libraries, and only when the target OS is Windows (MSVC, or MinGW with
`windres`).

## Output folders

| Option | Default | Receives |
|--------|---------|----------|
| `--output-path=path` | `.` | sets both bin and lib path |
| `--output-bin-path=path` | `.` | executables, DLLs / shared libraries |
| `--output-lib-path=path` | `.` | static libraries, import libraries, link copies of shared libraries |
| `--temp-path=path` | `./temp` | object files, response files, saved flags |

Folders are created when needed. `--output-path` resets both bin and lib
path, so give it before `--output-bin-path` / `--output-lib-path`.

## Tool chain selection

| Option | Effect |
|--------|--------|
| `--platform-compiler-msvc` | use MSVC |
| `--platform-compiler-gcc` | use GCC (also MinGW, Emscripten) |
| `--platform-64bit` / `--platform-32bit` | target bits (MSVC: `/MACHINE:X64` / `X86`, 32-bit also `/arch:SSE2`) |
| `--platform-os-windows` | target Windows |
| `--platform-os-linux` | target Linux |
| `--platform-os-emscripten` | target Emscripten (implies the Linux rules, compiler `emcc`) |
| `--platform=name` | platform name, default the one `xyo-cc` was built for (`XYO_PLATFORM_NAME`) |

The target is decided in this order, each step overriding the previous:

1. **Built in**: the compiler, bits and OS `xyo-cc` itself was compiled
   with.
2. **Environment**: `XYO_PLATFORM_COMPILER_MSVC`, `XYO_PLATFORM_COMPILER_GCC`,
   `XYO_PLATFORM_64BIT`, `XYO_PLATFORM_32BIT`, `XYO_PLATFORM_OS_LINUX`,
   `XYO_PLATFORM_OS_WINDOWS`, `XYO_PLATFORM_OS_EMSCRIPTEN` — any value, only
   presence counts. They can only *add* to the built-in choice; when two
   conflict, MSVC wins over GCC, 64-bit over 32-bit, Windows over Linux and
   Emscripten.
3. **Platform name**: if `--platform` contains `win64-msvc` or
   `win32-msvc`, the target is MSVC on Windows, 64 or 32-bit.
4. **Command line**: `--platform-compiler-*`, `--platform-64bit` /
   `--platform-32bit`, `--platform-os-*` replace the decision for their
   group (last one wins).
5. Emscripten implies Linux.

Because environment variables cannot remove a built-in choice, switching a
Windows build of `xyo-cc` to GCC or to Emscripten needs the command line
options: `--platform-compiler-gcc --platform-os-emscripten`.

If no compiler is selected: `Error: no compiler, use
--platform-compiler-msvc or --platform-compiler-gcc`.

## Argument files

`@file` arguments are expanded before the options are processed, in place:

- **`@name.json`**: a JSON project description, see below.
- **`@anything-else`**: a text file of arguments, split like a shell command
  line: separated by spaces, tabs or new lines; `"..."` groups an argument
  with spaces; `\"` is a literal quote; other backslashes are kept, so
  Windows paths work. There are no comments.

Expansion is one level deep: an `@file` inside an argument file is not
expanded and stops with `Error: unknown argument - @...`. A missing
file: `Error: file not found - <file>`, exit 1.

### JSON project description

The top level is an object; each known key becomes one or more options.
Unknown keys are **ignored** (so a `fabricare` compile file may carry extra
data).

| Key | Value | Becomes |
|-----|-------|---------|
| `project` | string | `--project=` |
| `type` | string: `exe`, `lib`, `dll`, `dll-x-static` | `--<type>` |
| `defines` | array of strings | `--def=` each |
| `includePath` | array of strings | `--inc=` each |
| `hSource` | array of strings | `--src-h=` each |
| `cSource` | array of strings | `--src-c=` each |
| `hppSource` | array of strings | `--src-hpp=` each |
| `cppSource` | array of strings | `--src-cpp=` each |
| `crt` | string: `dynamic`, `static` | `--crt-<crt>` |
| `libraryPath` | array of strings | `--use-lib-path=` each |
| `library` | array of strings | `--use-lib=` each |
| `linkerDefinitionsFile` | string | `--def-file=` |
| `resources` | object, keys below | |
| `resources.defines` | array of strings | `--rc-def=` each |
| `resources.includePath` | array of strings | `--rc-inc=` each |
| `resources.rcSource` | array of strings | `--rc-src=` each |

A key with the wrong type (a string where an array is expected, a number in
an array, ...) stops with `Error: json syntax - <key> - <file>`; an unknown
key inside `resources` with `Error: json syntax - resources/items - <file>`.
Output paths, `--debug`, `--threads` and the platform options have no JSON
key: pass them on the command line next to the `@file.json`.

```json
{
	"project": "tool",
	"type": "exe",
	"defines": ["TOOL_FEATURE_X"],
	"includePath": ["source", "../xyo-system/source"],
	"cppSource": ["source/XYO/Tool.Amalgam.cpp"],
	"libraryPath": ["output/lib"],
	"library": [":xyo-system"],
	"resources": {
		"includePath": ["source"],
		"rcSource": ["source/XYO/Tool/Application.rc"]
	},
	"crt": "static"
}
```

```bash
xyo-cc @tool.json --debug --output-bin-path=output/bin
```

## Environment variables

| Variable | Effect |
|----------|--------|
| `CC` | C compiler command (default `cl.exe`, `gcc`, or `emcc` for Emscripten) |
| `CXX` | C++ compiler and GCC linker command (default `cl.exe`, `gcc`, or `emcc`) |
| `XYO_PLATFORM_COMPILE_DEBUG` | `1`, `ON` or `TRUE`: debug build by default (`--release` still overrides) |
| `XYO_PLATFORM_COMPILE_CRT_STATIC` | `1`, `ON` or `TRUE`: static CRT by default (`--crt-dynamic` still overrides) |
| `XYO_PLATFORM_COMPILE_DEFINE` | space separated defines added before the command line ones |
| `XYO_PLATFORM_COMPILE_STATIC` | `1`, `ON` or `TRUE`: static build of everything — defines `XYO_PLATFORM_COMPILE_STATIC`, executables are compiled as static library users, GCC links `:name` libraries as `.a` |
| `XYO_PLATFORM_COMPILER_*`, `XYO_PLATFORM_64BIT`, `XYO_PLATFORM_32BIT`, `XYO_PLATFORM_OS_*` | tool chain selection, see above |

The value of `CC` / `CXX` is part of the saved compile flags: changing it
rebuilds the project.

## Exit code and output

- `0`: success (also for `--help`, `--version`, `--license` and no
  arguments).
- `1`: an error, with a one-line `Error: ...` message on standard output,
  for example `Error: building executable hello` after a failed compiler
  or linker call (whose own messages are printed above it).

Every compiler call is echoed as `[k/n] <compiler> @"<response file>"`, every
librarian / linker / resource compiler call as its command line. An up to
date project prints nothing.
