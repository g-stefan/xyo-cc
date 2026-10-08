# API reference

Namespace `XYO::CPPCompilerCommandDriver` unless noted. The compiler classes
are declared by `#include <XYO/CPPCompilerCommandDriver.hpp>`, the
application by `#include <XYO/CPPCompilerCommandDriver.Application.hpp>`.

## Macros

| Macro | Meaning |
|-------|---------|
| `XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT` | dllexport while building the DLL (`XYO_CPPCOMPILERCOMMANDDRIVER_DLL_INTERNAL`), dllimport for users, empty with `XYO_CPPCOMPILERCOMMANDDRIVER_LIBRARY` |
| `XYO_CPPCOMPILERCOMMANDDRIVER_INTERNAL` / `_DLL_INTERNAL` | defined when compiling this library; the build defines `XYO_CC_INTERNAL` / `XYO_CC_DLL_INTERNAL` (from the project name `xyo-cc`), mapped to these in `Dependency.hpp` |
| `XYO_CC_LIB_INTERNAL` | compiling the library as static library: exports empty |
| `XYO_CPPCOMPILERCOMMANDDRIVER_LIBRARY` | static linking, set for users of `xyo-cc.static` |
| `XYO_CPPCOMPILERCOMMANDDRIVER_APPLICATION_LIBRARY` | set for users of `xyo-cc.application.static`: the application is compiled without `main` |
| `XYO_CPPCOMPILERCOMMANDDRIVER_NO_VERSION`, `XYO_CPPCOMPILERCOMMANDDRIVER_APPLICATION_NO_VERSION` | build without the generated version information (amalgam builds in `input/`) |

## struct CompilerType

`CompilerType.hpp`

| Constant | Value |
|----------|-------|
| `None` | `0` |
| `MSVC` | `1` |
| `GCC` | `2` |

## struct CompilerOptions

`CompilerOptions.hpp`

| Constant | Value | Meaning |
|----------|-------|---------|
| `None` | `0` | |
| `Release` | `1` | optimized |
| `Debug` | `2` | debug info, no optimization |
| `CRTDynamic` | `4` | dynamic C runtime |
| `CRTStatic` | `8` | static C runtime |
| `StaticLibrary` | `16` | static library / uses static XYO libraries |
| `DynamicLibrary` | `32` | DLL / uses XYO DLLs |
| `DynamicLibraryXStatic` | `64` | DLL with static XYO dependencies |

```cpp
int filterOptions(int options);
```

Normalizes a mask: exactly one of `Release` / `Debug` (default `Release`),
one of `CRTDynamic` / `CRTStatic` (default `CRTDynamic`), one of
`StaticLibrary` / `DynamicLibrary` (default `DynamicLibrary`).
`DynamicLibraryXStatic` yields `DynamicLibrary | CRTStatic |
DynamicLibraryXStatic`.

## class ICompiler

`ICompiler.hpp` — `class ICompiler : public virtual Object`, interface
(`XYO_PLATFORM_INTERFACE`: not copyable).

```cpp
int type;              // CompilerType
bool isOSWindows;
bool isOSLinux;
bool isOSEmscripten;
bool is32Bit;
bool is64Bit;
bool isStatic;         // GCC: link ":name" libraries as .a
```

Pure virtual methods, all returning `bool` (`true` = success) except
`objFilename`:

```cpp
String objFilename(const String &project, const String &fileName, const String &tmpPath,
                   int options, int index, int indexLn);

bool cppToObj(int options, String cppFile, String objFile,
              TDynamicArray<String> &cppDefine, TDynamicArray<String> &incPath,
              int index, int indexLn, bool echoCmd);

bool cToObj(String cFile, String objFile, int options,
            TDynamicArray<String> &cDefine, TDynamicArray<String> &incPath,
            int index, int indexLn, bool echoCmd);

bool makeObjToLib(String libName, String binPath, String libPath, String tmpPath, int options,
                  TDynamicArray<String> &objFiles, String defFile,
                  TDynamicArray<String> &libDependencyPath, TDynamicArray<String> &libDependency,
                  String version, bool echoCmd, bool force = false);

bool makeObjToExe(String exeName, String binPath, String tmpPath, int options,
                  TDynamicArray<String> &objFiles,
                  TDynamicArray<String> &libDependencyPath, TDynamicArray<String> &libDependency,
                  bool echoCmd, bool force = false);

bool rcToRes(String rcFile, String resFile,
             TDynamicArray<String> &rcDefine, TDynamicArray<String> &incPath, bool echoCmd);

bool resToObj(String resFile, String objFile, bool echoCmd);

bool makeRcToObj(String rcFile, String objFile,
                 TDynamicArray<String> &rcDefine, TDynamicArray<String> &incPath,
                 bool echoCmd, bool force = false);

bool makeCppToLib(String libName, String binPath, String libPath, String tmpPath, int options,
                  TDynamicArray<String> &cppDefine, TDynamicArray<String> &incPath,
                  TDynamicArray<String> &incFiles, TDynamicArray<String> &cppFiles,
                  TDynamicArray<String> &rcDefine, TDynamicArray<String> &incPathRC,
                  TDynamicArray<String> &rcFiles, String defFile,
                  TDynamicArray<String> &libDependencyPath, TDynamicArray<String> &libDependency,
                  String version, int numThreads, bool echoCmd, bool force = false);

bool makeCppToExe(String exeName, String binPath, String tmpPath, int options,
                  TDynamicArray<String> &cppDefine, TDynamicArray<String> &incPath,
                  TDynamicArray<String> &incFiles, TDynamicArray<String> &cppFiles,
                  TDynamicArray<String> &rcDefine, TDynamicArray<String> &incPathRC,
                  TDynamicArray<String> &rcFiles,
                  TDynamicArray<String> &libDependencyPath, TDynamicArray<String> &libDependency,
                  int numThreads, bool echoCmd, bool force = false);

bool makeCToLib(/* same parameters as makeCppToLib, C sources */);
bool makeCToExe(/* same parameters as makeCppToExe, C sources */);
```

| Method | Up to date check | Notes |
|--------|------------------|-------|
| `objFilename` | | `<tmpPath>/<project>.<index padded>.<tag>.<file name>.obj\|.o` |
| `cppToObj`, `cToObj` | none, always compiles | creates the object folder, writes the response file |
| `rcToRes`, `resToObj` | none | GCC: `false` unless `isOSWindows` |
| `makeRcToObj` | `.res` vs `.rc`, object vs `.res` | |
| `makeObjToLib` | output vs objects, response file content | `false` on empty `objFiles` |
| `makeObjToExe` | output vs objects, response file content | `false` on empty `objFiles` |
| `makeCppToLib`, `makeCToLib` | per source, flags | resources only for `DynamicLibrary` |
| `makeCppToExe`, `makeCToExe` | per source, flags | |

## class CompilerMSVC, class CompilerGCC

`CompilerMSVC.hpp`, `CompilerGCC.hpp` — `public virtual ICompiler`,
exported, all methods of `ICompiler` implemented. The constructors set
`type` and every `bool` field to `false`.

| | `CompilerMSVC` | `CompilerGCC` |
|-|----------------|---------------|
| C++ compiler | `CXX` or `cl.exe`, `/std:c++17 /TP` | `CXX` or `gcc` (`emcc` for Emscripten), `-std=gnu++17 -fpermissive` |
| C compiler | `CC` or `cl.exe`, `/TC` | `CC` or `gcc` / `emcc` |
| object extension | `.obj` | `.o` |
| static library | `lib.exe` -> `<name>.lib` | `ar qcs` -> `<name>.a` |
| DLL / shared library | `link.exe /DLL` -> `<name>[-ver].dll` + import `.lib` | `CXX -shared` -> `<name>.so[.ver]` / `<name>[-ver].dll`, copy in `libPath` |
| executable | `link.exe` -> `<name>.exe` | `CXX` -> `<name>[.exe]` |
| resources | `rc.exe` + `cvtres.exe` | `windres` (Windows only) |

Details: [build process](build-process.md#compiler-commands).

## namespace CompilerMake

`CompilerMake.hpp` — namespace `XYO::CPPCompilerCommandDriver::CompilerMake`.

```cpp
String optionsTag(int options);
String replaceSuffix(const String &value, const String &suffix, const String &newSuffix);
String commandFilename(const String &objFile);
String libraryName(const String &name);
bool isContentChanged(const String &fileName, const String &content);

bool compileSources(ICompiler *compiler, bool isC, const String &projectName, const String &tmpPath,
                    int options, TDynamicArray<String> &define, TDynamicArray<String> &incPath,
                    TDynamicArray<String> &incFiles, TDynamicArray<String> &srcFiles,
                    int numThreads, bool echoCmd, bool force, TDynamicArray<String> &objFiles);

bool compileResources(ICompiler *compiler, const String &projectName, const String &tmpPath,
                      int options, TDynamicArray<String> &incFiles,
                      TDynamicArray<String> &rcDefine, TDynamicArray<String> &incPathRC,
                      TDynamicArray<String> &rcFiles, bool echoCmd, bool force,
                      TDynamicArray<String> &objFiles);
```

| Function | Returns / does |
|----------|----------------|
| `optionsTag` | letters for `Release`/`Debug`, CRT, library kind of an already filtered mask: `"RSS"`, `"DDD"`, ... |
| `replaceSuffix` | `value` with `suffix` replaced by `newSuffix`; `value` unchanged if it does not end with `suffix` |
| `commandFilename` | the last `.` of the file name replaced by `2`: `x.cpp.o` -> `x.cpp2o`; `x` -> `x.2` |
| `libraryName` | `name` without a leading `lib` |
| `isContentChanged` | `true` if the file is missing, unreadable, or its content differs |
| `compileSources` | compiles the out of date `srcFiles` with `numThreads` threads, saves `<tmpPath>/<project>.<tag>.<c\|cpp>.flags`; appends the object names of all `srcFiles` to `objFiles` |
| `compileResources` | compiles the out of date `rcFiles`, saves `<tmpPath>/<project>.<tag>.rc.flags`; appends their object names to `objFiles`; `true` for an empty list |

## Library metadata

Namespaces `Version`, `Copyright`, `License` inside
`XYO::CPPCompilerCommandDriver` (library) and
`XYO::CPPCompilerCommandDriver::Application` (application):

```cpp
const char *Version::version();           // "9.0.0"
const char *Version::build();             // "16"
const char *Version::versionWithBuild();  // "9.0.0.16"
const char *Version::datetime();          // "2026-09-27 23:53:45"
const char *Copyright::copyright();
const char *Copyright::publisher();
const char *Copyright::company();
const char *Copyright::contact();
std::string License::license();
std::string License::shortLicense();
```

With `*_NO_VERSION` the version strings are `"0.0.0"`, `"0"`, `"0.0.0.0"`,
`"0000-00-00"`.

## class Application::Application

`CPPCompilerCommandDriver.Application/Application.hpp` — namespace
`XYO::CPPCompilerCommandDriver::Application`,
`class Application : public virtual IApplication`, not copyable.

| Member | Does |
|--------|------|
| `Application()` | |
| `int main(int cmdN, char *cmdS[])` | the `xyo-cc` command line ([command line](command-line.md)); `cmdS[0]` ignored; returns `0` / `1` |
| `void showUsage()` | print the usage |
| `void showLicense()` | print the license |
| `void showVersion()` | print `version X build N [date]` |
| `static void initMemory()` | initialize the `String` and `TDynamicArray<String>` pools |

`XYO_APPLICATION_MAIN(Application)` provides `main` for the `xyo-cc`
executable unless `XYO_CPPCOMPILERCOMMANDDRIVER_APPLICATION_LIBRARY` is
defined.
