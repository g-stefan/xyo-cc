// C++ Compiler Command Driver
// Copyright (c) 2020-2024 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2020-2024 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/CPPCompilerCommandDriver.hpp>
#include <XYO/CPPCompilerCommandDriver.Application/Application.hpp>
#include <XYO/CPPCompilerCommandDriver.Application/Copyright.hpp>
#include <XYO/CPPCompilerCommandDriver.Application/License.hpp>
#include <XYO/CPPCompilerCommandDriver.Application/Version.hpp>
#include <XYO/FileJSON.hpp>

namespace XYO::CPPCompilerCommandDriver::Application {

	void Application::showUsage() {
		printf("C++ Compiler Command Driver\n");
		showVersion();
		printf("%s\n\n", CPPCompilerCommandDriver::Application::Copyright::copyright());

		printf("%s",
		       "options:\n"
		       "    --help                    this info\n"
		       "    --usage                   this info\n"
		       "    --license                 show license\n"
		       "    --version                 show version\n"
		       "    --project=name            project name\n"
		       "    --platform=name           platform name\n"
		       "    --debug                   build debug version\n"
		       "    --release                 build release version\n"
		       "    --exe                     build executable (.exe)\n"
		       "    --lib                     build library (.lib)\n"
		       "    --dll                     build dynamic library (.dll)\n"
		       "    --dll-x-static            build dynamic library with static linking (.dll)\n"
		       "    --crt-dynamic             build using dynamic crt (default for dll)\n"
		       "    --crt-static              build using static crt (default for lib)\n"
		       "    --threads=count           specify number of threads to use\n"
		       "    --def=value               add value to definitions\n"
		       "    --inc=path                add path to include in search\n"
		       "    --src-h=file              add file as h source\n"
		       "    --src-c=file              add file as c source\n"
		       "    --src-hpp=file            add file as hpp source\n"
		       "    --src-cpp=file            add file as cpp source\n"
		       "    --use-lib-path=path       add path to library search\n"
		       "    --use-lib=library         add library to linker\n"
		       "    --def-file=file           use file for linker definitions (.dll)\n"
		       "    --rc-inc=path             add path to resource compiler include\n"
		       "    --rc-def=value            add value to resource compiler definitions\n"
		       "    --rc-src=file             add file as rc source\n"
		       "    --source-path=path        folder where source files (.hpp/.h/.cpp/.c) are stored, default ./\n"
		       "    --output-path=path        location to output folder, default ./\n"
		       "    --temp-path=path          location to temp folder, default to ./temp\n"
		       "    --output-bin-path=path    location to output bin folder, default to ./output\n"
		       "    --output-lib-path=path    location to output lib folder, default to ./output\n"
		       "    --lib-name=name           use name for static library\n"
		       "    --lib-version=version     library name use version\n"
		       "    --force-make              force build all\n"
		       "    --no-lib                  do not generate library files (.lib), when build dll\n");
		printf("\n");
	};

	void Application::showLicense() {
		printf("%s", CPPCompilerCommandDriver::Application::License::license().c_str());
	};

	void Application::showVersion() {
		printf("version %s build %s [%s]\n", CPPCompilerCommandDriver::Application::Version::version(), CPPCompilerCommandDriver::Application::Version::build(), CPPCompilerCommandDriver::Application::Version::datetime());
	};

	void Application::initMemory() {
		String::initMemory();
		TDynamicArray<String>::initMemory();
	};

	int Application::main(int cmdN, char *cmdS[]) {
		int i;
		String opt;
		size_t optIndex;
		String optValue;
		TDynamicArray<String> cmdLine;

		// ---

		String projectName = "project";
		int crtOption = CompilerOptions::CRTDynamic;
		int dllOption = CompilerOptions::None;

		bool makeExecutable = false;
		bool makeLibrary = false;
		bool makeDynamicLibrary = false;
		bool isRelease = true;

		int numThreads = Processor::getCount();
		TDynamicArray<String> cppDefine;
		TDynamicArray<String> incPath;
		TDynamicArray<String> srcH;
		TDynamicArray<String> srcC;
		TDynamicArray<String> srcHpp;
		TDynamicArray<String> srcCpp;
		TDynamicArray<String> libDependencyPath;
		TDynamicArray<String> libDependency;
		String defFile;

		TDynamicArray<String> rcDefine;
		TDynamicArray<String> incPathRC;
		TDynamicArray<String> srcRc;

		String sourcePath = ".";
		String outputPath = ".";
		String tempPath = "./temp";
		String outputBinPath = outputPath;
		String outputLibPath = outputPath;
		String libName;
		String libVersion;
		String platformName = XYO_PLATFORM;

		bool forceMake = false;
		bool noLib = false;

		// ---

		if (Shell::hasEnv("XYO_COMPILE_DEBUG")) {
			isRelease = false;
		};

		Shell::getEnv("XYO_COMPILE_DEFINE").explode(" ", cppDefine);

		// ---

		for (i = 1; i < cmdN; ++i) {
			if (StringCore::beginWith(cmdS[i], "@")) {
				String content;
				if (StringCore::endsWith(cmdS[i], ".json")) {
					TPointer<FileJSON::Value> json;
					if (!FileJSON::load(&cmdS[i][1], json)) {
						printf("Error: json file load - %s\n", &cmdS[i][1]);
						return 1;
					};
					FileJSON::VAssociativeArray *jsonInfo = TDynamicCast<FileJSON::VAssociativeArray *>(json);
					if (!jsonInfo) {
						printf("Error: json no info - %s\n", &cmdS[i][1]);
						return 1;
					};
					int k, m, j;
					String key;
					FileJSON::Value *item;
					FileJSON::VString *vString;
					FileJSON::VArray *vArray;
					FileJSON::VAssociativeArray *vAssociativeArray;
					for (k = 0; k < jsonInfo->value->length(); ++k) {
						key = jsonInfo->value->arrayKey->index(k);
						item = jsonInfo->value->arrayValue->index(k);
						if (key == "project") {
							vString = TDynamicCast<FileJSON::VString *>(item);
							if (vString) {
								cmdLine.push(String("--project=") + vString->value);
								continue;
							};
							printf("Error: json syntax - project - %s\n", &cmdS[i][1]);
							return 1;
						};
						if (key == "type") {
							vString = TDynamicCast<FileJSON::VString *>(item);
							if (vString) {
								cmdLine.push(String("--") + vString->value);
								continue;
							};
							printf("Error: json syntax - type - %s\n", &cmdS[i][1]);
							return 1;
						};
						if (key == "defines") {
							vArray = TDynamicCast<FileJSON::VArray *>(item);
							if (vArray) {
								for (m = 0; m < vArray->value->length(); ++m) {
									vString = TDynamicCast<FileJSON::VString *>(vArray->value->index(m));
									if (vString) {
										cmdLine.push(String("--def=") + vString->value);
										continue;
									};
									printf("Error: json syntax - defines/items -  %s\n", &cmdS[i][1]);
									return 1;
								};
								continue;
							};
							printf("Error: json syntax - defines - %s\n", &cmdS[i][1]);
							return 1;
						};
						if (key == "includePath") {
							vArray = TDynamicCast<FileJSON::VArray *>(item);
							if (vArray) {
								for (m = 0; m < vArray->value->length(); ++m) {
									vString = TDynamicCast<FileJSON::VString *>(vArray->value->index(m));
									if (vString) {
										cmdLine.push(String("--inc=") + vString->value);
										continue;
									};
									printf("Error: json syntax - includePath/items - %s\n", &cmdS[i][1]);
									return 1;
								};
								continue;
							};
							printf("Error: json syntax - includePath - %s\n", &cmdS[i][1]);
							return 1;
						};
						if (key == "hSource") {
							vArray = TDynamicCast<FileJSON::VArray *>(item);
							if (vArray) {
								for (m = 0; m < vArray->value->length(); ++m) {
									vString = TDynamicCast<FileJSON::VString *>(vArray->value->index(m));
									if (vString) {
										cmdLine.push(String("--src-h=") + vString->value);
										continue;
									};
									printf("Error: json syntax - hSource/items - %s\n", &cmdS[i][1]);
									return 1;
								};
								continue;
							};
							printf("Error: json syntax - hSource - %s\n", &cmdS[i][1]);
							return 1;
						};
						if (key == "cSource") {
							vArray = TDynamicCast<FileJSON::VArray *>(item);
							if (vArray) {
								for (m = 0; m < vArray->value->length(); ++m) {
									vString = TDynamicCast<FileJSON::VString *>(vArray->value->index(m));
									if (vString) {
										cmdLine.push(String("--src-c=") + vString->value);
										continue;
									};
									printf("Error: json syntax - cSource/items - %s\n", &cmdS[i][1]);
									return 1;
								};
								continue;
							};
							printf("Error: json syntax - cSource - %s\n", &cmdS[i][1]);
							return 1;
						};
						if (key == "hppSource") {
							vArray = TDynamicCast<FileJSON::VArray *>(item);
							if (vArray) {
								for (m = 0; m < vArray->value->length(); ++m) {
									vString = TDynamicCast<FileJSON::VString *>(vArray->value->index(m));
									if (vString) {
										cmdLine.push(String("--src-hpp=") + vString->value);
										continue;
									};
									printf("Error: json syntax - hppSource/items - %s\n", &cmdS[i][1]);
									return 1;
								};
								continue;
							};
							printf("Error: json syntax - hppSource - %s\n", &cmdS[i][1]);
							return 1;
						};
						if (key == "cppSource") {
							vArray = TDynamicCast<FileJSON::VArray *>(item);
							if (vArray) {
								for (m = 0; m < vArray->value->length(); ++m) {
									vString = TDynamicCast<FileJSON::VString *>(vArray->value->index(m));
									if (vString) {
										cmdLine.push(String("--src-cpp=") + vString->value);
										continue;
									};
									printf("Error: json syntax - cppSource/items - %s\n", &cmdS[i][1]);
									return 1;
								};
								continue;
							};
							printf("Error: json syntax - cppSource - %s\n", &cmdS[i][1]);
							return 1;
						};
						if (key == "resources") {
							vAssociativeArray = TDynamicCast<FileJSON::VAssociativeArray *>(item);
							if (vAssociativeArray) {
								String vKey;
								FileJSON::Value *vItem;
								for (j = 0; j < vAssociativeArray->value->length(); ++j) {
									vKey = vAssociativeArray->value->arrayKey->index(j);
									vItem = vAssociativeArray->value->arrayValue->index(j);
									if (vKey == "defines") {
										vArray = TDynamicCast<FileJSON::VArray *>(vItem);
										if (vArray) {
											for (m = 0; m < vArray->value->length(); ++m) {
												vString = TDynamicCast<FileJSON::VString *>(vArray->value->index(m));
												if (vString) {
													cmdLine.push(String("--rc-def=") + vString->value);
													continue;
												};
												printf("Error: json syntax - resources - defines/items - %s\n", &cmdS[i][1]);
												return 1;
											};
											continue;
										};
										printf("Error: json syntax - resources - defines - %s\n", &cmdS[i][1]);
										return 1;
									};
									if (vKey == "includePath") {
										vArray = TDynamicCast<FileJSON::VArray *>(vItem);
										if (vArray) {
											for (m = 0; m < vArray->value->length(); ++m) {
												vString = TDynamicCast<FileJSON::VString *>(vArray->value->index(m));
												if (vString) {
													cmdLine.push(String("--rc-inc=") + vString->value);
													continue;
												};
												printf("Error: json syntax - resources - includePath/items - %s\n", &cmdS[i][1]);
												return 1;
											};
											continue;
										};
										printf("Error: json syntax - resources - includePath - %s\n", &cmdS[i][1]);
										return 1;
									};
									if (vKey == "rcSource") {
										vArray = TDynamicCast<FileJSON::VArray *>(vItem);
										if (vArray) {
											for (m = 0; m < vArray->value->length(); ++m) {
												vString = TDynamicCast<FileJSON::VString *>(vArray->value->index(m));
												if (vString) {
													cmdLine.push(String("--rc-src=") + vString->value);
													continue;
												};
												printf("Error: json syntax - resources - rcSource/items - %s\n", &cmdS[i][1]);
												return 1;
											};
											continue;
										};
										printf("Error: json syntax - resources - rcSource - %s\n", &cmdS[i][1]);
										return 1;
									};
									printf("Error: json syntax - resources/items - %s\n", &cmdS[i][1]);
									return 1;
								};
								continue;
							};
							printf("Error: json syntax - resources - %s\n", &cmdS[i][1]);
							return 1;
						};
						if (key == "crt") {
							vString = TDynamicCast<FileJSON::VString *>(item);
							if (vString) {
								cmdLine.push(String("--crt-") + vString->value);
								continue;
							};
							printf("Error: json syntax - crt - %s\n", &cmdS[i][1]);
							return 1;
						};
						if (key == "libraryPath") {
							vArray = TDynamicCast<FileJSON::VArray *>(item);
							if (vArray) {
								for (m = 0; m < vArray->value->length(); ++m) {
									vString = TDynamicCast<FileJSON::VString *>(vArray->value->index(m));
									if (vString) {
										cmdLine.push(String("--use-lib-path=") + vString->value);
										continue;
									};
									printf("Error: json syntax - libraryPath/items - %s\n", &cmdS[i][1]);
									return 1;
								};
								continue;
							};
							printf("Error: json syntax - libraryPath - %s\n", &cmdS[i][1]);
							return 1;
						};
						if (key == "library") {
							vArray = TDynamicCast<FileJSON::VArray *>(item);
							if (vArray) {
								for (m = 0; m < vArray->value->length(); ++m) {
									vString = TDynamicCast<FileJSON::VString *>(vArray->value->index(m));
									if (vString) {
										cmdLine.push(String("--use-lib=") + vString->value);
										continue;
									};
									printf("Error: json syntax - library/items - %s\n", &cmdS[i][1]);
									return 1;
								};
								continue;
							};
							printf("Error: json syntax - library - %s\n", &cmdS[i][1]);
							return 1;
						};
						if (key == "linkerDefinitionsFile") {
							vString = TDynamicCast<FileJSON::VString *>(item);
							if (vString) {
								cmdLine.push(String("--def-file=") + vString->value);
								continue;
							};
							printf("Error: json syntax - linkerDefinitionsFile - %s\n", &cmdS[i][1]);
							return 1;
						};
					};
				};
				if (Shell::fileGetContents(&cmdS[i][1], content)) {
					int cmdNX;
					char **cmdSX;
					int m;
					Shell::mainArgsSet(content, cmdNX, cmdSX);
					for (m = 0; m < cmdNX; ++m) {
						cmdLine.push(cmdSX[m]);
					};
					Shell::mainArgsDelete(cmdNX, cmdSX);
					continue;
				};
				printf("Error: file not found - %s\n", &cmdS[i][1]);
				return 1;
			};
			cmdLine.push(cmdS[i]);
		};

		for (i = 0; i < cmdLine.length(); ++i) {
			if (StringCore::beginWith(cmdLine[i], "--")) {
				opt = cmdLine[i].index(2);
				optValue = "";
				if (opt.indexOf("=", 0, optIndex)) {
					optValue = opt.substring(optIndex + 1);
					opt = opt.substring(0, optIndex);
				};
				if (opt == "help") {
					showUsage();
					return 0;
				};
				if (opt == "usage") {
					showUsage();
					return 0;
				};
				if (opt == "license") {
					showLicense();
					return 0;
				};
				if (opt == "version") {
					showVersion();
					return 0;
				};
				if (opt == "project") {
					if (optValue.length() == 0) {
						printf("Error: project is empty\n");
						return 1;
					};
					projectName = optValue;
					continue;
				};
				if (opt == "platform") {
					if (optValue.length() == 0) {
						printf("Error: platform is empty\n");
						return 1;
					};
					platformName = optValue;
					continue;
				};
				if (opt == "debug") {
					isRelease = false;
					continue;
				};
				if (opt == "release") {
					isRelease = true;
					continue;
				};
				if (opt == "exe") {
					makeExecutable = true;
					continue;
				};
				if (opt == "lib") {
					makeLibrary = true;
					crtOption = CompilerOptions::CRTStatic;
					continue;
				};
				if (opt == "dll") {
					makeDynamicLibrary = true;
					crtOption = CompilerOptions::CRTDynamic;
					continue;
				};
				if (opt == "dll-x-static") {
					makeDynamicLibrary = true;
					crtOption = CompilerOptions::CRTStatic;
					dllOption = CompilerOptions::DynamicLibraryXStatic;
					continue;
				};
				if (opt == "crt-dynamic") {
					crtOption = CompilerOptions::CRTDynamic;
					continue;
				};
				if (opt == "crt-static") {
					crtOption = CompilerOptions::CRTStatic;
					continue;
				};
				if (opt == "threads") {
					if (sscanf(optValue.value(), "%d", &numThreads) != 1) {
						numThreads = Processor::getCount();
					};
					continue;
				};
				if (opt == "def") {
					if (optValue.isEmpty()) {
						printf("Error: def parameter is empty\n");
						return 1;
					};
					cppDefine.push(optValue);
					continue;
				};
				if (opt == "inc") {
					if (optValue.isEmpty()) {
						printf("Error: inc path is empty\n");
						return 1;
					};
					incPath.push(optValue);
					continue;
				};
				if (opt == "src-h") {
					if (optValue.isEmpty()) {
						printf("Error: src-h file not provided\n");
						return 1;
					};
					srcH.push(optValue);
					continue;
				};
				if (opt == "src-c") {
					if (optValue.isEmpty()) {
						printf("Error: src-c file not provided\n");
						return 1;
					};
					srcC.push(optValue);
					continue;
				};
				if (opt == "src-hpp") {
					if (optValue.isEmpty()) {
						printf("Error: src-hpp file not provided\n");
						return 1;
					};
					srcHpp.push(optValue);
					continue;
				};
				if (opt == "src-cpp") {
					if (optValue.isEmpty()) {
						printf("Error: src-cpp file not provided\n");
						return 1;
					};
					srcCpp.push(optValue);
					continue;
				};
				if (opt == "use-lib-path") {
					if (optValue.isEmpty()) {
						printf("Error: use-lib-path is empty\n");
						return 1;
					};
					libDependencyPath.push(optValue);
					continue;
				};
				if (opt == "use-lib") {
					if (optValue.isEmpty()) {
						printf("Error: use-lib is empty\n");
						return 1;
					};
					libDependency.push(optValue);
					if (libDependency[libDependency.length() - 1].endsWith(".static")) {
						crtOption = CompilerOptions::CRTStatic;
					};
					continue;
				};
				if (opt == "def-file") {
					if (optValue.isEmpty()) {
						printf("Error: def-file is empty\n");
						return 1;
					};
					defFile = optValue;
					continue;
				};
				if (opt == "rc-inc") {
					if (optValue.isEmpty()) {
						printf("Error: rc-inc path is empty\n");
						return 1;
					};
					incPathRC.push(optValue);
					continue;
				};
				if (opt == "rc-def") {
					if (optValue.isEmpty()) {
						printf("Error: rc-def parameter is empty\n");
						return 1;
					};
					rcDefine.push(optValue);
					continue;
				};
				if (opt == "rc-src") {
					if (optValue.isEmpty()) {
						printf("Error: rc-src file not provided\n");
						return 1;
					};
					srcRc.push(optValue);
					continue;
				};
				if (opt == "source-path") {
					if (optValue.isEmpty()) {
						printf("Error: source-path not provided\n");
						return 1;
					};
					sourcePath = optValue;
					continue;
				};
				if (opt == "output-path") {
					if (optValue.isEmpty()) {
						printf("Error: output-path not provided\n");
						return 1;
					};
					outputPath = optValue;
					outputBinPath = outputPath;
					outputLibPath = outputPath;
				};
				if (opt == "temp-path") {
					if (optValue.isEmpty()) {
						printf("Error: temp-path is empty\n");
						return 1;
					};
					tempPath = optValue;
					continue;
				};
				if (opt == "output-bin-path") {
					if (optValue.isEmpty()) {
						printf("Error: output-bin-path is empty\n");
						return 1;
					};
					outputBinPath = optValue;
					continue;
				};
				if (opt == "output-lib-path") {
					if (optValue.isEmpty()) {
						printf("Error: output-lib-path is empty\n");
						return 1;
					};
					outputLibPath = optValue;
					continue;
				};
				if (opt == "lib-name") {
					if (optValue.isEmpty()) {
						printf("Error: lib-name is empty\n");
						return 1;
					};
					libName = optValue;
					continue;
				};
				if (opt == "lib-version") {
					if (optValue.isEmpty()) {
						printf("Error: lib-version is empty\n");
						return 1;
					};
					libVersion = optValue;
					continue;
				};
				if (opt == "force-make") {
					forceMake = true;
					continue;
				};
				if (opt == "no-lib") {
					noLib = true;
					continue;
				};

				continue;
			};
		};

		if (cmdLine.length() == 0) {
			showUsage();
			return 0;
		};

		if ((!makeLibrary) && (!makeDynamicLibrary) && (!makeExecutable)) {
			printf("Error: no exe/dll/lib specified to compile\n");
			return 1;
		};

		// ---

		TDynamicArray<String> cSource;
		TDynamicArray<String> hFiles;
		TDynamicArray<String> cppSource;
		TDynamicArray<String> hppFiles;
		TDynamicArray<String> rcFiles;

		size_t k;

		for (k = 0; k < srcH.length(); ++k) {
			if (!Shell::fileExists(sourcePath + "/" + srcH[k])) {
				printf("Error: file not found %s\n", srcH[k].value());
				return 1;
			};
			hFiles.push(sourcePath + "/" + srcH[k]);
		};

		for (k = 0; k < srcC.length(); ++k) {
			if (!Shell::fileExists(sourcePath + "/" + srcC[k])) {
				printf("Error: file not found %s\n", srcC[k].value());
				return 1;
			};
			cSource.push(sourcePath + "/" + srcC[k]);
		};

		for (k = 0; k < srcHpp.length(); ++k) {
			if (!Shell::fileExists(sourcePath + "/" + srcHpp[k])) {
				printf("Error: file not found %s\n", srcHpp[k].value());
				return 1;
			};
			hppFiles.push(sourcePath + "/" + srcHpp[k]);
		};

		for (k = 0; k < srcCpp.length(); ++k) {
			if (!Shell::fileExists(sourcePath + "/" + srcCpp[k])) {
				printf("Error: file not found %s\n", srcCpp[k].value());
				return 1;
			};
			cppSource.push(sourcePath + "/" + srcCpp[k]);
		};

		for (k = 0; k < srcRc.length(); ++k) {
			if (!Shell::fileExists(sourcePath + "/" + srcRc[k])) {
				printf("Error: file not found %s\n", srcRc[k].value());
				return 1;
			};
			rcFiles.push(sourcePath + "/" + srcRc[k]);
		};

		// ---

		String defInternal = projectName.toUpperCaseAscii() + "_INTERNAL";
		defInternal = defInternal.replace("-", "_");
		defInternal = defInternal.replace(".", "_");
		if (projectName.beginWith("lib")) {
			defInternal = defInternal.substring(3);
		};
		cppDefine.push(defInternal);

		// ---

		TPointer<ICompiler> compiler;
#ifdef XYO_COMPILER_MSVC
		compiler = TMemory<CompilerMSVC>::newMemory();
#endif
#ifdef XYO_COMPILER_GCC
		compiler = TMemory<CompilerGCC>::newMemory();
#endif

#ifdef XYO_APPLICATION_64BIT
		compiler->is64Bit = true;
#else
		compiler->is64Bit = false;
#endif

#ifdef XYO_APPLICATION_32BIT
		compiler->is32Bit = true;
#else
		compiler->is32Bit = false;
#endif

#ifdef XYO_OS_LINUX
		compiler->isOSLinux = true;
#else
		compiler->isOSLinux = false;
#endif

#ifdef XYO_OS_WINDOWS
		compiler->isOSWindows = true;
#else
		compiler->isOSWindows = false;
#endif

		size_t strIndex;
		if (platformName.indexOf("win64-msvc", 0, strIndex)) {
			if (compiler->type != CompilerType::MSVC) {
				compiler = TMemory<CompilerMSVC>::newMemory();
			};
			compiler->isOSWindows = true;
			compiler->isOSLinux = false;
			compiler->is64Bit = true;
			compiler->is32Bit = false;
		};
		if (platformName.indexOf("win32-msvc", 0, strIndex)) {
			if (compiler->type != CompilerType::MSVC) {
				compiler = TMemory<CompilerMSVC>::newMemory();
			};
			compiler->isOSWindows = true;
			compiler->isOSLinux = false;
			compiler->is64Bit = false;
			compiler->is32Bit = true;
		};

		// ---

		if (makeLibrary) {
			if ((cSource.isEmpty()) && (cppSource.isEmpty())) {
				printf("Error: no c/cpp source for library %s\n", projectName.value());
				return 1;
			};
			if (cSource.length() > 0) {
				if (!compiler->makeCToLib(
				        libName.length() ? libName : projectName + ".static",
				        outputBinPath,
				        outputLibPath,
				        tempPath,
				        (isRelease ? CompilerOptions::Release : CompilerOptions::Debug) | crtOption | CompilerOptions::StaticLibrary,
				        cppDefine,
				        incPath,
				        hFiles,
				        cSource,
				        rcDefine,
				        incPathRC,
				        rcFiles,
				        defFile,
				        libDependencyPath,
				        libDependency,
				        libVersion,
				        numThreads,
				        true,
				        forceMake)) {
					printf("Error: building library %s\n", projectName.value());
					return 1;
				};
			};
			if (cppSource.length() > 0) {
				if (!compiler->makeCppToLib(
				        libName.length() ? libName : projectName + ".static",
				        outputBinPath,
				        outputLibPath,
				        tempPath,
				        (isRelease ? CompilerOptions::Release : CompilerOptions::Debug) | crtOption | CompilerOptions::StaticLibrary,
				        cppDefine,
				        incPath,
				        hppFiles,
				        cppSource,
				        rcDefine,
				        incPathRC,
				        rcFiles,
				        defFile,
				        libDependencyPath,
				        libDependency,
				        libVersion,
				        numThreads,
				        true,
				        forceMake)) {
					printf("Error: building library %s\n", projectName.value());
					return 1;
				};
			};
		};

		if (makeDynamicLibrary) {
			if (noLib) {
				outputLibPath = tempPath;
			};

			if ((cSource.isEmpty()) && (cppSource.isEmpty())) {
				printf("Error: no c/cpp source for dynamic library %s\n", projectName.value());
				return 1;
			};
			if (cSource.length() > 0) {
				if (!compiler->makeCToLib(
				        projectName,
				        outputBinPath,
				        outputLibPath,
				        tempPath,
				        (isRelease ? CompilerOptions::Release : CompilerOptions::Debug) | crtOption | CompilerOptions::DynamicLibrary | dllOption,
				        cppDefine,
				        incPath,
				        hFiles,
				        cSource,
				        rcDefine,
				        incPathRC,
				        rcFiles,
				        defFile,
				        libDependencyPath,
				        libDependency,
				        libVersion,
				        numThreads,
				        true,
				        forceMake)) {
					printf("Error: building dynamic library %s\n", projectName.value());
					return 1;
				};
			};

			if (cppSource.length() > 0) {
				if (!compiler->makeCppToLib(
				        projectName,
				        outputBinPath,
				        outputLibPath,
				        tempPath,
				        (isRelease ? CompilerOptions::Release : CompilerOptions::Debug) | crtOption | CompilerOptions::DynamicLibrary | dllOption,
				        cppDefine,
				        incPath,
				        hppFiles,
				        cppSource,
				        rcDefine,
				        incPathRC,
				        rcFiles,
				        defFile,
				        libDependencyPath,
				        libDependency,
				        libVersion,
				        numThreads,
				        true,
				        forceMake)) {
					printf("Error: building dynamic library %s\n", projectName.value());
					return 1;
				};
			};
		};

		if (makeExecutable) {
			if ((cSource.isEmpty()) && (cppSource.isEmpty())) {
				printf("Error: no c/cpp source for executable %s\n", projectName.value());
				return 1;
			};
			if (cSource.length() > 0) {
				if (!compiler->makeCToExe(
				        projectName,
				        outputBinPath,
				        tempPath,
				        (isRelease ? CompilerOptions::Release : CompilerOptions::Debug) | crtOption,
				        cppDefine,
				        incPath,
				        hFiles,
				        cSource,
				        rcDefine,
				        incPathRC,
				        rcFiles,
				        libDependencyPath,
				        libDependency,
				        numThreads,
				        true,
				        forceMake)) {
					printf("Error: building executable %s\n", projectName.value());
					return 1;
				};
			};

			if (cppSource.length() > 0) {
				if (!compiler->makeCppToExe(
				        projectName,
				        outputBinPath,
				        tempPath,
				        (isRelease ? CompilerOptions::Release : CompilerOptions::Debug) | crtOption,
				        cppDefine,
				        incPath,
				        hppFiles,
				        cppSource,
				        rcDefine,
				        incPathRC,
				        rcFiles,
				        libDependencyPath,
				        libDependency,
				        numThreads,
				        true,
				        forceMake)) {
					printf("Error: building executable %s\n", projectName.value());
					return 1;
				};
			};
		};
		return 0;
	};
};

#ifndef XYO_CPPCOMPILERCOMMANDDRIVER_APPLICATION_LIBRARY
XYO_APPLICATION_MAIN(XYO::CPPCompilerCommandDriver::Application::Application);
#endif
