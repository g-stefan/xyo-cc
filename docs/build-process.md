# Build process

What `xyo-cc` does with the options, step by step, and the exact files and
commands it produces. The examples use a project `hello` / `greet`; `<temp>`,
`<bin>` and `<lib>` are `--temp-path`, `--output-bin-path` and
`--output-lib-path`.

## Steps

1. **Read the arguments**: expand `@file` / `@file.json`, apply the
   environment defaults, parse the options
   ([command line](command-line.md)).
2. **Check the sources**: every `--src-*` / `--rc-src` file must exist
   under `--source-path`. If C and C++ sources are mixed, the C sources join
   the C++ list.
3. **Add the project defines** (below).
4. **Select the compiler**: `CompilerMSVC` or `CompilerGCC`, with the
   target OS and bits ([tool chain selection](command-line.md#tool-chain-selection)).
5. **For each target** — library, then DLL, then executable:
   1. compile the sources that are out of date, in parallel;
   2. for executables and DLLs on Windows, compile the resources that are
      out of date;
   3. archive or link, if any object file is newer than the output or the
      link command changed.

The first failure stops the build with exit code 1.

## Defines added by xyo-cc

On top of `--def` and `XYO_PLATFORM_COMPILE_DEFINE`:

| Define | When | Example (`--project=xyo-cc`) |
|--------|------|------------------------------|
| `<PROJECT>_INTERNAL` | always | `XYO_CC_INTERNAL` |
| `<PROJECT>_LIB_INTERNAL` | `--lib` | `XYO_CC_LIB_INTERNAL` |
| `<PROJECT>_DLL_INTERNAL` | `--dll`, `--dll-x-static` | `XYO_CC_DLL_INTERNAL` |
| `<PROJECT>_EXE_INTERNAL` | `--exe` | `XYO_CC_EXE_INTERNAL` |
| `XYO_PLATFORM_COMPILE_STATIC` | env `XYO_PLATFORM_COMPILE_STATIC` is `1` / `ON` / `TRUE` | |

`<PROJECT>` is the project name in upper case with `-` and `.` replaced by
`_`; a leading `lib` is dropped (`libfoo` -> `FOO_INTERNAL`). Only one of
the `_LIB_` / `_DLL_` / `_EXE_` defines is added: when several targets are
built in one call, `_EXE_` wins over `_DLL_` over `_LIB_`, for all of them.

These are what the XYO export macros test. A library `my-lib` declares

```cpp
#ifdef MY_LIB_DLL_INTERNAL
#	define MY_LIB_EXPORT XYO_PLATFORM_LIBRARY_EXPORT
#else
#	define MY_LIB_EXPORT XYO_PLATFORM_LIBRARY_IMPORT
#endif
```

and is exported while `xyo-cc --dll --project=my-lib` builds it, imported
everywhere else. When the code uses a different prefix than the project
name, map it in `Dependency.hpp`; this repository maps `XYO_CC_*` to
`XYO_CPPCOMPILERCOMMANDDRIVER_*`.

The compiler driver adds the build description, on every source:

| Define | Set for |
|--------|---------|
| `XYO_PLATFORM_COMPILE_RELEASE` / `XYO_PLATFORM_COMPILE_DEBUG` | `--release` / `--debug` |
| `XYO_PLATFORM_COMPILE_CRT_DYNAMIC` / `XYO_PLATFORM_COMPILE_CRT_STATIC` | `--crt-dynamic` / `--crt-static` (and `--dll-x-static`) |
| `XYO_PLATFORM_COMPILE_STATIC_LIBRARY` | `--lib`, `--dll-x-static`, `--exe` when `XYO_PLATFORM_COMPILE_STATIC` is set |
| `XYO_PLATFORM_COMPILE_DYNAMIC_LIBRARY` | `--dll`, `--exe` (default) |

For an executable, "dynamic library" means it uses the XYO libraries as
DLLs, the default; with `XYO_PLATFORM_COMPILE_STATIC` it links them
statically.

## Incremental build

A source is compiled when **any** of these holds:

- its object file is missing;
- the object file is older than the source;
- the object file is older than **any** `--src-hpp` / `--src-h` file;
- the compile flags of the project changed since the last successful build;
- `--force-make`.

`xyo-cc` does not scan `#include` lines: the headers it watches are exactly
the ones listed. Listing all headers of a project makes a change to any of
them rebuild the whole project, which is simple and safe.

**Compile flags** are saved after a successful compile step in
`<temp>/<project>.<tag>.<c|cpp|rc>.flags`, one file per language. They
record the compiler kind, the language, the options, the target OS / bits /
static flags, the `CC` or `CXX` value, every define and every include path:

```
xyo-cc.flags.1
msvc
cpp
RSD
W---6-

DX=1
DHELLO_INTERNAL
DHELLO_EXE_INTERNAL
Isource
```

Change any of them and all sources of that language are rebuilt.

**Linking** (and archiving) happens when the output is missing, any object
file is newer than the output, or the response file of the link would have
a different content than the one saved from the last link. After a failed
compiler or linker call the response file is removed, so the next run tries
again.

**Resources** are compiled `.rc` -> `.res` when the `.res` is missing or
older than the `.rc`, then `.res` -> object when the object is missing or
older than the `.res`; also when any listed header is newer than the object,
or the resource flags changed.

## Parallel compilation

Out of date sources are queued on a `WorkerQueue` (`xyo-multithreading`)
with `--threads` threads, the number of processors by default. Each
compilation runs its compiler with `Shell::system`, so the compilers run in
parallel and their messages can interleave. If any compilation fails, the
build stops after the queue finishes. Resources are compiled one by one.

## File names

`<tag>` is three letters for the effective options: `R`elease / `D`ebug,
then CRT `S`tatic / `D`ynamic, then `S`tatic library / `D`ynamic library.
`RSS` = release, static CRT, static library; `RDD` = release, dynamic CRT,
DLL or executable using DLLs.

`<k>` is the 1-based index of the file in its list, zero padded to the
number of digits of the list length (`1`, or `01` .. `12`).

| File | MSVC | GCC |
|------|------|-----|
| object of a source | `<temp>/<project><ext>.<k>.<tag>.<source name>.obj` | `<temp>/<project><ext>.<k>.<tag>.<source name>.o` |
| compiler response file | object name with the last `.` replaced by `2`: `hello.cpp.obj` -> `hello.cpp2obj` | `hello.cpp.o` -> `hello.cpp2o` |
| saved flags | `<temp>/<project><ext>.<tag>.cpp.flags` (`.c.flags`, `.rc.flags`) | same |
| object of a resource | `<temp>/<project><ext>.<k>.<tag>.<name>.rc.obj` + `.rc.res` | `... .rc.o` + `.rc.res` |
| static library | `<lib>/<name>.lib`, response `<temp>/<name>.obj2lib` | `<lib>/<name>.a`, response `<temp>/<name>.o2a` |
| DLL / shared library | `<bin>/<name>[-<version>].dll` + import `<lib>/<name>.lib`, response `<temp>/<name>.obj2dll` | Linux: `<bin>/<name>.so[.<version>]`, MinGW: `<bin>/<name>[-<version>].dll`; copied to `<lib>/<name>.so` / `.dll` for linking; response `<temp>/<name>.o2so` |
| executable | `<bin>/<name>.exe`, response `<temp>/<name>.obj2exe` | `<bin>/<name>` (`.exe` on Windows), response `<temp>/<name>.o2elf` |

`<ext>` keeps the object files of different targets of the same project
apart: `.lib` / `.dll` / `.exe` with MSVC, `.a` / `.so` / nothing with GCC
(also `.so` on MinGW). `<name>` is the project name, or `--lib-name` for a
static library. There is no `lib` prefix on GCC outputs: link them with
`--use-lib=:name` or `-l:name.a`.

Example, MSVC, `--exe --project=hello --crt-static` with one source:

```
temp/hello.exe.1.RSD.hello.cpp.obj
temp/hello.exe.1.RSD.hello.cpp2obj
temp/hello.exe.RSD.cpp.flags
temp/hello.obj2exe
hello.exe
```

## Compiler commands

Every compile writes its arguments to the response file and runs
`<compiler> @"<response file>"`; open the response file to see the full
command.

### MSVC

| Step | Command and main flags |
|------|------------------------|
| C++ source | `cl.exe /nologo /std:c++17 /TP` (or `CXX`) |
| C source | `cl.exe /nologo /TC` (or `CC`) |
| release | `/O2 /Oi /Oy /Gy /Gd /EHsc /GR /c`, `/MD` or `/MT` |
| debug | `/Zi /EHsc /GR /c`, `/MDd` or `/MTd`, `/Fd<object>.pdb` |
| 32-bit | `/arch:SSE2` |
| defines, includes | `/D"..."`, `/I"..."` |
| static library | `lib.exe /NOLOGO /OUT:... /MACHINE:X64\|X86 <objects>` |
| DLL | `link.exe /NOLOGO /DLL /ENTRY:_DllMainCRTStartup[@12] /MACHINE:... [/DEF:...] /implib:<lib>/<name>.lib` |
| executable | `link.exe /NOLOGO /OUT:<bin>/<name>.exe /MACHINE:...` |
| link options | `/INCREMENTAL:NO /OPT:REF /OPT:ICF`, `/RELEASE` or `/DEBUG`, the matching `/defaultlib:` `msvcrt[d]` / `libcmt[d]` |
| libraries | `/LIBPATH:"..."`, `<name>.lib`, then always `user32 gdi32 ws2_32 ole32 wininet advapi32 oleaut32 shell32 iphlpapi` |
| resources | `rc.exe /nologo /i ... /d ... /l 409 /r /fo <res> <rc>`, then `cvtres.exe /NOLOGO /MACHINE:... /OUT:<obj> <res>` |

Paths are written with `\`.

### GCC, MinGW, Emscripten

| Step | Command and main flags |
|------|------------------------|
| C++ source | `gcc` (or `CXX`; `emcc` for Emscripten) `-std=gnu++17 -fpermissive`; a `.c` file in the C++ list gets `-x c++` |
| C source | `gcc` (or `CC`; `emcc`) |
| release / debug | `-O2` / `-O0 -g` |
| DLL / executable using DLLs | `-fpic`, plus `-rdynamic` on Linux (not Emscripten) |
| Emscripten | `-pthread` |
| defines, includes | `-D"..."`, `-I"..."` |
| static library | `ar qcs <lib>/<name>.a @<response file>` (all objects in one call) |
| shared library | `gcc -shared -o <output> -Wl,-rpath='$ORIGIN'[,-soname,...] <objects> <libraries> -lstdc++ -lpthread -lm` + `-ldl` (Linux) / `-luser32 -lws2_32` (Windows) |
| executable | `gcc -o <output> -Wl,-rpath='$ORIGIN' <objects> <libraries>` + `-lstdc++ -lpthread -lm` and `-ldl` (Linux) / `-luser32 -lws2_32` (Windows); MinGW with `--crt-static`: `-static-libstdc++ -static-libgcc` and the C++ runtime linked statically |
| resources (MinGW) | `windres -I ... --define ... -l 409 -J rc -O res`, then `windres -J res -O coff` |

Paths are written with `/`. `-rpath='$ORIGIN'` makes an executable find its
shared libraries in its own folder.

## Linking libraries

`--use-lib-path` adds search paths. `--use-lib` names a library in one of
two forms:

| `--use-lib=` | MSVC | GCC |
|--------------|------|-----|
| `name` (system or third party library) | `name.lib` | `-lname`, a leading `lib` removed (`libz` -> `-lz`) |
| `:name` (XYO library, by file name) | `name.lib` | `-l:name.so` (Linux), `-l:name.dll` (MinGW); `-l:name.a` when `XYO_PLATFORM_COMPILE_STATIC` is set |
| `:name.static` | `name.static.lib` | `-l:name.static.a` |

The `:` form exists because GCC outputs of `xyo-cc` have no `lib` prefix;
`-l:file` makes the GCC linker look for the exact file name. `fabricare`
passes the `dependency` entries of a `fabricare.json` in this form
(`:xyo-system`, `:file-json.static`) and system libraries in the plain form.

Libraries are passed in the order given. With GCC, list a library after the
libraries that use it.

## Emscripten

`--platform-os-emscripten` (the target also counts as Linux):

- the compiler and linker default to `emcc`;
- objects are compiled with `-pthread`; libraries and executables are
  linked with `-s NODERAWFS=1 -pthread` (direct access to the host file
  system under Node.js);
- an executable gets `--pre-js <temp>/<name>.prerun.js`, which copies
  `process.platform` to `ENV['EMSCRIPTEN_PLATFORM']`, `process.env.TEMP` to
  `ENV['TEMP']`, and every variable named in `<temp>/<name>.env` (one name
  per line, `#` comments, blank lines ignored) from the Node.js environment
  into the program's environment. A change of the generated script relinks
  the executable.
