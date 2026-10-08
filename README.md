# C++ Compiler Command Driver

Command line tool and C++ library
- Describe a C/C++ project once (sources, defines, include paths, libraries, resources)
and build it with MSVC or GCC (also MinGW and Emscripten).
- Executables, static libraries and DLLs / shared libraries, release or debug,
dynamic or static C runtime.
- Incremental and parallel: only out of date sources are compiled, a change of the
compile flags rebuilds, a change of the link command relinks.
- Arguments on the command line, in `@file` argument files or in `@file.json`
project descriptions.

Built on `xyo-system` and `file-json`; `fabricare` builds every XYO C++ project through it.

```bash
xyo-cc --exe --project=hello --inc=source --src-cpp=source/hello.cpp --output-bin-path=output/bin
```

## Documentation

- [Overview](docs/README.md) - purpose and design
- [Getting started](docs/getting-started.md) - build, first builds, argument files, use from fabricare
- [Command line](docs/command-line.md) - every option, `@file` / `@file.json`, environment variables, tool chain selection
- [Build process](docs/build-process.md) - defines added, incremental rules, file names, MSVC / GCC commands, linking
- [Library](docs/library.md) - `ICompiler`, `CompilerMSVC`, `CompilerGCC` from C++, embedding the application
- [API reference](docs/reference.md)

A Claude Code skill for this project is in
[.claude/skills/xyo-cc](.claude/skills/xyo-cc/SKILL.md).

## License

Copyright (c) 2020-2026 Grigore Stefan
Licensed under the [MIT](LICENSE) license.
