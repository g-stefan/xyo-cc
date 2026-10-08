# Library

The command line tool is a thin layer over a C++ library, namespace
`XYO::CPPCompilerCommandDriver`. Use the library when a program of yours
must compile C / C++ code — a build tool, a script host, a code generator
that builds what it generates — without spawning `xyo-cc` itself.

## Depend on it

In the consumer's `fabricare.json`:

```json
{
	"name": "my-builder",
	"make": "exe",
	"sourcePath": "XYO/MyBuilder",
	"dependency": [
		"xyo-cc"
	]
}
```

| Dependency | You get |
|------------|---------|
| `xyo-cc` | the compiler classes, as DLL / shared library |
| `xyo-cc.static` | the compiler classes, static; exports `XYO_CPPCOMPILERCOMMANDDRIVER_LIBRARY` to the consumer |
| `xyo-cc.application.static` | the classes plus the command line application, static; exports `XYO_CPPCOMPILERCOMMANDDRIVER_APPLICATION_LIBRARY`, which removes its `main` |

`xyo-system`, `file-json` (application only) and everything below come in
as transitive dependencies.

```cpp
#include <XYO/CPPCompilerCommandDriver.hpp>              // compiler classes
#include <XYO/CPPCompilerCommandDriver.Application.hpp>  // + Application
```

The namespace has `using namespace XYO::System`, so `String`,
`TDynamicArray`, `TPointer`, `TMemory`, `Shell`, ... are visible through it.
C++17, multithreaded build (the compile step uses `WorkerQueue`).

## Build a program with a compiler object

```cpp
#include <XYO/CPPCompilerCommandDriver.hpp>

using namespace XYO::CPPCompilerCommandDriver;

bool buildHello() {
	TPointer<ICompiler> compiler;
#ifdef XYO_PLATFORM_COMPILER_MSVC
	compiler = TMemory<CompilerMSVC>::newMemory();
#else
	compiler = TMemory<CompilerGCC>::newMemory();
#endif

	// target: the constructor sets every flag to false
#ifdef XYO_PLATFORM_OS_WINDOWS
	compiler->isOSWindows = true;
#else
	compiler->isOSLinux = true;
#endif
#ifdef XYO_PLATFORM_64BIT
	compiler->is64Bit = true;
#else
	compiler->is32Bit = true;
#endif

	TDynamicArray<String> cppDefine;
	TDynamicArray<String> incPath;
	TDynamicArray<String> incFiles;  // headers: dependencies of every source
	TDynamicArray<String> cppFiles;
	TDynamicArray<String> rcDefine;
	TDynamicArray<String> incPathRC;
	TDynamicArray<String> rcFiles;
	TDynamicArray<String> libDependencyPath;
	TDynamicArray<String> libDependency;

	cppDefine.push("HELLO_INTERNAL");
	incPath.push("source");
	incFiles.push("source/hello.hpp");
	cppFiles.push("source/hello.cpp");
	cppFiles.push("source/main.cpp");

	return compiler->makeCppToExe(
	    "hello",        // exeName
	    "output/bin",   // binPath
	    "temp",         // tmpPath
	    CompilerOptions::Release | CompilerOptions::CRTDynamic | CompilerOptions::DynamicLibrary,
	    cppDefine,
	    incPath,
	    incFiles,
	    cppFiles,
	    rcDefine,
	    incPathRC,
	    rcFiles,
	    libDependencyPath,
	    libDependency,
	    Processor::getCount(), // numThreads
	    true,                  // echoCmd: print the commands
	    false);                // force: rebuild everything
};
```

This is the same build as

```bash
xyo-cc --exe --project=hello --inc=source --src-hpp=source/hello.hpp --src-cpp=source/hello.cpp --src-cpp=source/main.cpp --output-bin-path=output/bin --temp-path=temp
```

except for what only the application does (next section).

### What the library does and does not do

The library does, inside each `make*` call: incremental and parallel
compilation, saved flags, resources, archive / link, the
`XYO_PLATFORM_COMPILE_*` defines of the options, `CC` / `CXX`, folder
creation, path separators.

The **application** does, before calling the library — do it yourself when
you need it:

- add `<PROJECT>_INTERNAL` and `<PROJECT>_LIB|DLL|EXE_INTERNAL`;
- read `XYO_PLATFORM_COMPILE_DEBUG`, `_CRT_STATIC`, `_DEFINE`, `_STATIC`
  and the tool chain selection variables;
- check that the source files exist and prefix them with `--source-path`;
- move C sources into the C++ list when both are present (or call
  `makeCToExe` / `makeCToLib` for pure C);
- set `isStatic` (from `XYO_PLATFORM_COMPILE_STATIC`);
- choose `StaticLibrary` / `DynamicLibrary` for executables.

## The interface

`ICompiler` (`Object`, managed, not copyable) has two implementations,
`CompilerMSVC` and `CompilerGCC`. Hold them in a `TPointer<ICompiler>`.

### Target fields

| Field | Meaning |
|-------|---------|
| `int type` | `CompilerType::MSVC` or `CompilerType::GCC`, set by the constructor |
| `bool isOSWindows`, `isOSLinux`, `isOSEmscripten` | target OS; for Emscripten set `isOSLinux` too |
| `bool is32Bit`, `is64Bit` | target bits; MSVC uses them for `/MACHINE` and the DLL entry point |
| `bool isStatic` | GCC: link `:name` libraries as `.a` |

GCC needs one of `isOSLinux` / `isOSWindows` to build a shared library and
to add the system libraries of an executable; MSVC resources and linking
work regardless.

### Options

`options` is a bit mask of `CompilerOptions`:

| Bit | Value |
|-----|-------|
| `Release` / `Debug` | `1` / `2` |
| `CRTDynamic` / `CRTStatic` | `4` / `8` |
| `StaticLibrary` / `DynamicLibrary` | `16` / `32` |
| `DynamicLibraryXStatic` | `64` |

Every method normalizes it with `filterOptions(options)`: `Debug` else
`Release`, `CRTStatic` else `CRTDynamic`, `StaticLibrary` else
`DynamicLibrary`; `DynamicLibraryXStatic` forces `DynamicLibrary |
CRTStatic`. So `CompilerOptions::None` means release, dynamic CRT, dynamic
library. For `makeCppToLib` / `makeCToLib` the library bit selects the
output: `StaticLibrary` archives, `DynamicLibrary` links a DLL / shared
library. For executables it only selects the defines (`DynamicLibrary` =
uses XYO DLLs).

### Whole builds

| Method | Builds |
|--------|--------|
| `makeCppToExe(exeName, binPath, tmpPath, options, cppDefine, incPath, incFiles, cppFiles, rcDefine, incPathRC, rcFiles, libDependencyPath, libDependency, numThreads, echoCmd, force)` | executable from C++ sources |
| `makeCppToLib(libName, binPath, libPath, tmpPath, options, cppDefine, incPath, incFiles, cppFiles, rcDefine, incPathRC, rcFiles, defFile, libDependencyPath, libDependency, version, numThreads, echoCmd, force)` | static library or DLL from C++ sources |
| `makeCToExe(...)`, `makeCToLib(...)` | the same for C sources |

Parameters:

- `exeName` / `libName`: output name without extension.
- `binPath`, `libPath`, `tmpPath`: output folders, created when missing
  (see [file names](build-process.md#file-names)).
- `cppDefine` / `cDefine`, `incPath`: `NAME[=value]` defines and include
  paths.
- `incFiles`: headers whose time stamps invalidate every object.
- `cppFiles` / `cFiles`: sources, as paths usable from the current folder.
- `rcDefine`, `incPathRC`, `rcFiles`: resources; used for executables and
  DLLs (GCC: only when `isOSWindows`).
- `defFile`: MSVC module definition file for a DLL, or empty.
- `libDependencyPath`, `libDependency`: as `--use-lib-path` / `--use-lib`
  ([linking libraries](build-process.md#linking-libraries)).
- `version`: DLL / shared library version suffix, or empty.
- `numThreads`: parallel compilations.
- `echoCmd`: print each command before running it.
- `force`: ignore time stamps and saved flags.

They return `true` on success. On failure the tool's own messages are on
the console; the method itself prints nothing more. The arrays are passed
by reference but not modified.

### Single steps

| Method | Does |
|--------|------|
| `objFilename(project, fileName, tmpPath, options, index, indexLn)` | object file name for a source |
| `cppToObj(options, cppFile, objFile, cppDefine, incPath, index, indexLn, echoCmd)` | compile one C++ source, always |
| `cToObj(cFile, objFile, options, cDefine, incPath, index, indexLn, echoCmd)` | compile one C source, always (note the parameter order differs from `cppToObj`) |
| `makeObjToLib(libName, binPath, libPath, tmpPath, options, objFiles, defFile, libDependencyPath, libDependency, version, echoCmd, force)` | archive or link a library, if out of date |
| `makeObjToExe(exeName, binPath, tmpPath, options, objFiles, libDependencyPath, libDependency, echoCmd, force)` | link an executable, if out of date |
| `rcToRes(rcFile, resFile, rcDefine, incPath, echoCmd)` | compile a resource script, always |
| `resToObj(resFile, objFile, echoCmd)` | convert a `.res` to an object, always |
| `makeRcToObj(rcFile, objFile, rcDefine, incPath, echoCmd, force)` | both, if out of date |

`index` / `indexLn` only number the echoed line (`[index/indexLn]`).
`makeObjTo*` fail on an empty object list. The GCC resource methods fail
unless `isOSWindows`.

### CompilerMake

`XYO::CPPCompilerCommandDriver::CompilerMake` holds the steps shared by
both compilers, used by the `make*` methods and exported for reuse:

| Function | Does |
|----------|------|
| `compileSources(compiler, isC, projectName, tmpPath, options, define, incPath, incFiles, srcFiles, numThreads, echoCmd, force, objFiles)` | compile the out of date sources in parallel, save the flags; `objFiles` receives the object names of **all** sources |
| `compileResources(compiler, projectName, tmpPath, options, incFiles, rcDefine, incPathRC, rcFiles, echoCmd, force, objFiles)` | the same for resources, sequential |
| `optionsTag(options)` | `"RSS"`, `"RDD"`, ... |
| `commandFilename(objFile)` | response file name: `x.cpp.o` -> `x.cpp2o` |
| `replaceSuffix(value, suffix, newSuffix)` | replace a suffix, unchanged if absent |
| `libraryName(name)` | drop a leading `lib` |
| `isContentChanged(fileName, content)` | true if the file is missing, unreadable or different |

With them, a custom pipeline is: `compileSources` -> add your own object
files -> `makeObjToLib` / `makeObjToExe`.

## Run the command line tool in process

Link `xyo-cc.application.static` and call `Application::main` with an
`argv` exactly as on the command line (`argv[0]` is ignored):

```cpp
#include <XYO/CPPCompilerCommandDriver.Application.hpp>

int runXYOCC(TDynamicArray<String> &arguments) {
	TDynamicArray<String> cmdV;
	cmdV.push("xyo-cc");
	for (size_t k = 0; k < arguments.length(); ++k) {
		cmdV.push(arguments[k]);
	};
	int cmdN = (int)cmdV.length();
	char **cmdS = new char *[cmdN + 1];
	for (int k = 0; k < cmdN; ++k) {
		cmdS[k] = const_cast<char *>(cmdV[k].value());
	};
	cmdS[cmdN] = nullptr;
	int retV;
	{
		XYO::CPPCompilerCommandDriver::Application::Application application;
		retV = application.main(cmdN, cmdS); // 0 = success
	};
	delete[] cmdS;
	return retV;
};
```

```cpp
TDynamicArray<String> arguments;
arguments.push("@temp/tool.compile.json");
arguments.push("--exe");
arguments.push("--output-bin-path=output/bin");
if (runXYOCC(arguments) != 0) {
	// failed, messages already printed
};
```

This is how `fabricare` provides `xyoCC(...)` to its scripts. The
application keeps no state between calls. Its pools for `String` and
`TDynamicArray<String>` are set up by the static
`Application::initMemory()`; call it from your own application's
`initMemory()` if your program does not already initialize those types.
