# Getting started

## 1. Build and install

`xyo-cc` is built with [fabricare](https://github.com/g-stefan/fabricare),
the build tool of all XYO C++ projects. `xyo-platform`, `xyo-managed-memory`,
`xyo-data-structures`, `xyo-multithreading`, `xyo-encoding`, `xyo-system` and
`file-json` must be installed to the SDK first. From the repository root:

```bash
fabricare make       # build into output/
fabricare test       # xyo-cc builds itself from the amalgam sources (run make first)
fabricare install    # copy output/{bin,include,lib} to ~/.fabricare/<platform>
fabricare clean      # remove output/ and temp/
```

`fabricare.json` defines four projects:

| Project                    | Kind                                | Use it when                                                  |
|----------------------------|-------------------------------------|--------------------------------------------------------------|
| `xyo-cc` (library)         | DLL / shared library (`dll-or-lib`) | you call the compiler classes from your own program          |
| `xyo-cc.static`            | static library, static CRT          | the same, linked into a self-contained executable            |
| `xyo-cc.application.static`| static library, static CRT          | you embed the whole command line tool (as `fabricare` does)  |
| `xyo-cc` (executable)      | `xyo-cc` / `xyo-cc.exe`             | the command line tool                                        |

After `fabricare install` the tool is in `~/.fabricare/<platform>/bin`,
which is on the `PATH` of an XYO development machine.

Check it:

```bash
xyo-cc --version
```

```
version 9.0.0 build 16 [2026-09-27 23:53:45]
```

## 2. The tool chain must be on the PATH

`xyo-cc` only writes command lines; the compilers must be callable:

- **MSVC**: run it from a *Developer Command Prompt* or after
  `vcvars64.bat` / `vcvars32.bat`, so `cl.exe`, `lib.exe`, `link.exe`,
  `rc.exe` and `cvtres.exe` are found.
- **GCC**: `gcc` (used for C, C++ and linking, with `-lstdc++` added) or
  the commands in the `CC` / `CXX` environment variables, plus `ar`, and
  `windres` for resources on MinGW.
- **Emscripten**: `emcc`, or `CC` / `CXX`.

By default `xyo-cc` uses the tool chain it was itself built with (a
`win64-msvc` build drives MSVC 64-bit, a Linux build drives GCC). See
[tool chain selection](command-line.md#tool-chain-selection) to change it.

## 3. First build: an executable

```cpp
// source/hello.cpp
#include <stdio.h>

int main() {
	printf("hello\n");
	return 0;
};
```

```bash
xyo-cc --exe --project=hello --src-cpp=source/hello.cpp
```

```
[1/1] cl.exe @".\temp\hello.exe.1.RSD.hello.cpp2obj"
link.exe @".\temp\hello.obj2exe"
```

`hello.exe` (or `hello` on Linux) is written to the current folder, the
object and response files to `./temp`. Run the same command again and
nothing happens: the object file is newer than the source and the flags did
not change. Add `--debug`, a `--def=...` or another `--inc=...` and the
project is rebuilt.

Typical layout, with separate output folders:

```bash
xyo-cc --exe --project=hello --inc=source --src-hpp=source/hello.hpp --src-cpp=source/hello.cpp --src-cpp=source/main.cpp --output-bin-path=output/bin --temp-path=temp
```

List the headers with `--src-hpp` / `--src-h`: they are not compiled, but a
header newer than an object file makes that object rebuild.

## 4. A static library and an executable that links it

```bash
xyo-cc --lib --project=greet --crt-static --inc=source --src-hpp=source/greet.hpp --src-cpp=source/greet.cpp --output-lib-path=output/lib
```

```bash
xyo-cc --exe --project=hello --crt-static --inc=source --src-hpp=source/greet.hpp --src-cpp=source/hello.cpp --use-lib-path=output/lib --use-lib=greet --output-bin-path=output/bin
```

The library is `output/lib/greet.lib` with MSVC, `output/lib/greet.a` with
GCC (no `lib` prefix). `--use-lib=greet` links it on both. Keep the C runtime
option the same for a static library and everything that links it.

A DLL / shared library:

```bash
xyo-cc --dll --project=greet --inc=source --src-cpp=source/greet.cpp --output-bin-path=output/bin --output-lib-path=output/lib
```

MSVC: `output/bin/greet.dll` and the import library `output/lib/greet.lib`.
GCC on Linux: `output/bin/greet.so` and a copy `output/lib/greet.so` to link
against. Your code must export its symbols; with XYO conventions it tests
`GREET_DLL_INTERNAL`, defined by `xyo-cc` while it builds the DLL
([defines added](build-process.md#defines-added-by-xyo-cc)).

## 5. The same with argument files

Long command lines go into a file, one or more arguments per line:

```
--lib
--project=greet
--crt-static
--inc=source
--src-hpp=source/greet.hpp
--src-cpp=source/greet.cpp
```

```bash
xyo-cc @greet.arguments --output-lib-path=output/lib
```

Or a JSON project description (the format `fabricare` generates):

```json
{
	"project": "greet",
	"type": "lib",
	"includePath": ["source"],
	"hppSource": ["source/greet.hpp"],
	"cppSource": ["source/greet.cpp"],
	"crt": "static"
}
```

```bash
xyo-cc @greet.json --output-lib-path=output/lib
```

Arguments from files and from the command line are processed in order and
can be mixed; [command line](command-line.md#argument-files) has the details
and the full list of JSON keys. [`input/`](../input/) has a complete example
of each form: `xyo-cc` building itself and the XYO stack from the
amalgamated sources.

## 6. From fabricare

In an XYO project you normally do not call `xyo-cc` yourself. For each
project of a `"type": "xyo-cpp"` solution, `fabricare make`:

1. collects the sources of `sourcePath`, the dependencies and their defines;
2. writes `temp/<project>.compile.json`;
3. calls the embedded `xyo-cc` with
   `@temp/<project>.compile.json --exe|--lib|--dll --output-...-path=...`,
   plus `--inc` / `--use-lib-path` / `--rc-inc` for `output/`, the SDK
   (`~/.fabricare/<platform>`) and the parent folder, and `--debug` when
   fabricare runs with `--debug`.

So the `temp/*.compile.json` files of any XYO project show exactly what was
passed to `xyo-cc`, and you can rerun one by hand:

```bash
xyo-cc @temp/xyo-cc.compile.json --exe --output-bin-path=output/bin
```

In fabricare scripts the tool is available as `xyoCC(...)`, with the same
arguments:

```js
exitIf(xyoCC("--exe", "--project=tool", "--src-cpp=source/tool.cpp", "--output-bin-path=output/bin"));
```

`xyoCC` returns the exit code (0 on success).

## 7. Building xyo-cc without fabricare

`xyo-cc` builds itself from the amalgamated sources of the stack. With an
existing `xyo-cc` and the sibling repositories checked out next to this one
(`../xyo-platform`, `../xyo-system`, `../file-json`, ...):

```bash
xyo-cc @input/xyo-cc-x.compile.arguments --output-bin-path=output/test
```

```bash
xyo-cc @input/xyo-cc-y.compile.json --output-bin-path=output/test
```

This is what `fabricare test` runs. Without any `xyo-cc`, compile the same
eight `*.Amalgam.cpp` files listed in those files directly with the
compiler, with the same defines (`XYO_PLATFORM_COMPILE_AMALGAM`,
`XYO_*_NO_VERSION`) and include paths, as C++17.

## 8. Calling it from C++

Two ways, both shown in [Library](library.md):

- create a `CompilerMSVC` or `CompilerGCC`, set its target flags and call
  `makeCppToExe` / `makeCppToLib` / ... with lists of files;
- run the command line tool in process:
  `XYO::CPPCompilerCommandDriver::Application::Application().main(argc, argv)`,
  linked from `xyo-cc.application.static`.
