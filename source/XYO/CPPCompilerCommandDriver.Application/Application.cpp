// C++ Compiler Command Driver
// Copyright (c) 2020-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2020-2026 Grigore Stefan <g_stefan@yahoo.com>
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
		       "    --crt-dynamic             build using dynamic crt (default)\n"
		       "    --crt-static              build using static crt\n"
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
		       "    --output-bin-path=path    location to output bin folder, default to output-path\n"
		       "    --output-lib-path=path    location to output lib folder, default to output-path\n"
		       "    --lib-name=name           use name for static library\n"
		       "    --lib-version=version     library name use version\n"
		       "    --force-make              force build all\n"
		       "    --no-lib                  do not generate library files (.lib), when build dll\n"
		       "    --platform-compiler-msvc  use msvc compiler\n"
		       "    --platform-compiler-gcc   use gcc compiler\n"
		       "    --platform-64bit          compile for 64bit\n"
		       "    --platform-32bit          compile for 32bit\n"
		       "    --platform-os-linux       platform os is linux\n"
		       "    --platform-os-windows     platform os is windows\n"
		       "    --platform-os-emscripten  platform os is emscripten\n");
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

	// JSON key to command line option
	struct JSONOption {
			const char *key;
			const char *prefix;
			bool isArray;
	};

	static const JSONOption jsonOptions[] = {
	    {"project", "--project=", false},
	    {"type", "--", false},
	    {"defines", "--def=", true},
	    {"includePath", "--inc=", true},
	    {"hSource", "--src-h=", true},
	    {"cSource", "--src-c=", true},
	    {"hppSource", "--src-hpp=", true},
	    {"cppSource", "--src-cpp=", true},
	    {"crt", "--crt-", false},
	    {"libraryPath", "--use-lib-path=", true},
	    {"library", "--use-lib=", true},
	    {"linkerDefinitionsFile", "--def-file=", false}};

	static const JSONOption jsonResourcesOptions[] = {
	    {"defines", "--rc-def=", true},
	    {"includePath", "--rc-inc=", true},
	    {"rcSource", "--rc-src=", true}};

	static const JSONOption *findJSONOption(const JSONOption *options, size_t optionsLength, const String &key) {
		size_t k;
		for (k = 0; k < optionsLength; ++k) {
			if (key == options[k].key) {
				return &options[k];
			};
		};
		return nullptr;
	};

	// String, or array of strings, to command line options
	static bool pushJSONOption(const JSONOption &option, FileJSON::Value *item, TDynamicArray<String> &cmdLine) {
		FileJSON::VString *vString;
		if (!option.isArray) {
			vString = TDynamicCast<FileJSON::VString *>(item);
			if (!vString) {
				return false;
			};
			cmdLine.push(String(option.prefix) + vString->value);
			return true;
		};
		FileJSON::VArray *vArray = TDynamicCast<FileJSON::VArray *>(item);
		if (!vArray) {
			return false;
		};
		size_t k;
		for (k = 0; k < vArray->value->length(); ++k) {
			vString = TDynamicCast<FileJSON::VString *>(vArray->value->index(k));
			if (!vString) {
				return false;
			};
			cmdLine.push(String(option.prefix) + vString->value);
		};
		return true;
	};

	static bool loadJSONArguments(const char *fileName, TDynamicArray<String> &cmdLine) {
		TPointer<FileJSON::Value> json;
		if (!FileJSON::load(fileName, json)) {
			printf("Error: json file load - %s\n", fileName);
			return false;
		};
		FileJSON::VAssociativeArray *jsonInfo = TDynamicCast<FileJSON::VAssociativeArray *>(json);
		if (!jsonInfo) {
			printf("Error: json no info - %s\n", fileName);
			return false;
		};
		size_t k, m;
		String key;
		FileJSON::Value *item;
		const JSONOption *option;
		for (k = 0; k < jsonInfo->value->length(); ++k) {
			key = jsonInfo->value->arrayKey->index(k);
			item = jsonInfo->value->arrayValue->index(k);
			if (key == "resources") {
				FileJSON::VAssociativeArray *resources = TDynamicCast<FileJSON::VAssociativeArray *>(item);
				if (!resources) {
					printf("Error: json syntax - resources - %s\n", fileName);
					return false;
				};
				String resourcesKey;
				for (m = 0; m < resources->value->length(); ++m) {
					resourcesKey = resources->value->arrayKey->index(m);
					option = findJSONOption(jsonResourcesOptions, sizeof(jsonResourcesOptions) / sizeof(jsonResourcesOptions[0]), resourcesKey);
					if (!option) {
						printf("Error: json syntax - resources/items - %s\n", fileName);
						return false;
					};
					if (!pushJSONOption(*option, resources->value->arrayValue->index(m), cmdLine)) {
						printf("Error: json syntax - resources - %s - %s\n", resourcesKey.value(), fileName);
						return false;
					};
				};
				continue;
			};
			option = findJSONOption(jsonOptions, sizeof(jsonOptions) / sizeof(jsonOptions[0]), key);
			if (!option) {
				// unknown keys are ignored
				continue;
			};
			if (!pushJSONOption(*option, item, cmdLine)) {
				printf("Error: json syntax - %s - %s\n", key.value(), fileName);
				return false;
			};
		};
		return true;
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
		String platformName = XYO_PLATFORM_NAME;

		bool optPlatformCompilerMSVC = false;
		bool optPlatformCompilerGCC = false;
		bool optPlatform64bit = false;
		bool optPlatform32bit = false;
		bool optPlatformOSLinux = false;
		bool optPlatformOSWindows = false;
		bool optPlatformOSEmscripten = false;

		// selected on the command line, overrides the build and environment defaults, last one wins
		int cmdPlatformCompiler = CompilerType::None;
		int cmdPlatformBits = 0;
		String cmdPlatformOS;

		bool forceMake = false;
		bool noLib = false;

		// ---

#ifdef XYO_PLATFORM_COMPILER_MSVC
		optPlatformCompilerMSVC = true;
#endif
#ifdef XYO_PLATFORM_COMPILER_GCC
		optPlatformCompilerGCC = true;
#endif

#ifdef XYO_PLATFORM_64BIT
		optPlatform64bit = true;
#endif
#ifdef XYO_PLATFORM_32BIT
		optPlatform32bit = true;
#endif

#ifdef XYO_PLATFORM_OS_LINUX
		optPlatformOSLinux = true;
#endif
#ifdef XYO_PLATFORM_OS_WINDOWS
		optPlatformOSWindows = true;
#endif
#ifdef XYO_PLATFORM_OS_EMSCRIPTEN
		optPlatformOSEmscripten = true;
#endif

		if (Shell::hasEnv("XYO_PLATFORM_COMPILER_MSVC")) {
			optPlatformCompilerMSVC = true;
		};
		if (Shell::hasEnv("XYO_PLATFORM_COMPILER_GCC")) {
			optPlatformCompilerGCC = true;
		};
		if (Shell::hasEnv("XYO_PLATFORM_64BIT")) {
			optPlatform64bit = true;
		};
		if (Shell::hasEnv("XYO_PLATFORM_32BIT")) {
			optPlatform32bit = true;
		};
		if (Shell::hasEnv("XYO_PLATFORM_OS_LINUX")) {
			optPlatformOSLinux = true;
		};
		if (Shell::hasEnv("XYO_PLATFORM_OS_WINDOWS")) {
			optPlatformOSWindows = true;
		};
		if (Shell::hasEnv("XYO_PLATFORM_OS_EMSCRIPTEN")) {
			optPlatformOSEmscripten = true;
		};

		// ---

		if (Shell::hasEnv("XYO_PLATFORM_COMPILE_DEBUG")) {
			String env = Shell::getEnv("XYO_PLATFORM_COMPILE_DEBUG");
			if (env == "1" || env == "ON" || env == "TRUE") {
				isRelease = false;
			};
		};

		if (Shell::hasEnv("XYO_PLATFORM_COMPILE_CRT_STATIC")) {
			String env = Shell::getEnv("XYO_PLATFORM_COMPILE_CRT_STATIC");
			if (env == "1" || env == "ON" || env == "TRUE") {
				crtOption = CompilerOptions::CRTStatic;
			};
		};

		{
			TDynamicArray<String> envDefine;
			Shell::getEnv("XYO_PLATFORM_COMPILE_DEFINE").explode(" ", envDefine);
			for (i = 0; i < envDefine.length(); ++i) {
				// empty on repeated spaces
				if (envDefine[i].isEmpty()) {
					continue;
				};
				cppDefine.push(envDefine[i]);
			};
		};

		// ---

		for (i = 1; i < cmdN; ++i) {
			if (StringCore::beginWith(cmdS[i], "@")) {
				if (StringCore::endsWith(cmdS[i], ".json")) {
					if (!loadJSONArguments(&cmdS[i][1], cmdLine)) {
						return 1;
					};
					continue;
				};
				String content;
				if (Shell::fileGetContents(&cmdS[i][1], content)) {
					XYO::System::ShellArguments shellArguments;
					int m;
					shellArguments.set(content);
					for (m = 0; m < shellArguments.cmdN; ++m) {
						cmdLine.push(shellArguments.cmdS[m]);
					};
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
					continue;
				};
				if (opt == "dll") {
					makeDynamicLibrary = true;
					continue;
				};
				if (opt == "dll-x-static") {
					makeDynamicLibrary = true;
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
					continue;
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

				if (opt == "platform-compiler-msvc") {
					cmdPlatformCompiler = CompilerType::MSVC;
					continue;
				};
				if (opt == "platform-compiler-gcc") {
					cmdPlatformCompiler = CompilerType::GCC;
					continue;
				};
				if (opt == "platform-64bit") {
					cmdPlatformBits = 64;
					continue;
				};
				if (opt == "platform-32bit") {
					cmdPlatformBits = 32;
					continue;
				};
				if (opt == "platform-os-linux") {
					cmdPlatformOS = "linux";
					continue;
				};
				if (opt == "platform-os-windows") {
					cmdPlatformOS = "windows";
					continue;
				};
				if (opt == "platform-os-emscripten") {
					cmdPlatformOS = "emscripten";
					continue;
				};

				printf("Error: unknown option - %s\n", cmdLine[i].value());
				return 1;
			};
			printf("Error: unknown argument - %s\n", cmdLine[i].value());
			return 1;
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

		String defInternal = projectName.toUpperCaseASCII() + "_INTERNAL";
		defInternal = defInternal.replace("-", "_");
		defInternal = defInternal.replace(".", "_");
		if (projectName.beginWith("lib")) {
			defInternal = defInternal.substring(3);
		};
		cppDefine.push(defInternal);

		// ---

		defInternal = "";
		if (makeLibrary) {
			defInternal = projectName.toUpperCaseASCII() + "_LIB_INTERNAL";
		};
		if (makeDynamicLibrary) {
			defInternal = projectName.toUpperCaseASCII() + "_DLL_INTERNAL";
		};
		if (makeExecutable) {
			defInternal = projectName.toUpperCaseASCII() + "_EXE_INTERNAL";
		};
		if (defInternal.length() > 0) {
			defInternal = defInternal.replace("-", "_");
			defInternal = defInternal.replace(".", "_");
			if (projectName.beginWith("lib")) {
				defInternal = defInternal.substring(3);
			};
			cppDefine.push(defInternal);
		};

		// --- platform: build defaults, environment, platform name, command line

		if (optPlatformCompilerMSVC) {
			optPlatformCompilerGCC = false;
		};
		if (optPlatform64bit) {
			optPlatform32bit = false;
		};
		if (optPlatformOSWindows) {
			optPlatformOSLinux = false;
			optPlatformOSEmscripten = false;
		};
		if (optPlatformOSLinux) {
			optPlatformOSWindows = false;
		};

		size_t strIndex;
		if (platformName.indexOf("win64-msvc", 0, strIndex) || platformName.indexOf("win32-msvc", 0, strIndex)) {
			optPlatformCompilerMSVC = true;
			optPlatformCompilerGCC = false;
			optPlatformOSWindows = true;
			optPlatformOSLinux = false;
			optPlatformOSEmscripten = false;
			optPlatform64bit = platformName.indexOf("win64-msvc", 0, strIndex);
			optPlatform32bit = !optPlatform64bit;
		};

		if (cmdPlatformCompiler != CompilerType::None) {
			optPlatformCompilerMSVC = (cmdPlatformCompiler == CompilerType::MSVC);
			optPlatformCompilerGCC = (cmdPlatformCompiler == CompilerType::GCC);
		};
		if (cmdPlatformBits != 0) {
			optPlatform64bit = (cmdPlatformBits == 64);
			optPlatform32bit = (cmdPlatformBits == 32);
		};
		if (!cmdPlatformOS.isEmpty()) {
			optPlatformOSWindows = (cmdPlatformOS == "windows");
			optPlatformOSLinux = (cmdPlatformOS == "linux");
			optPlatformOSEmscripten = (cmdPlatformOS == "emscripten");
		};

		if (optPlatformOSEmscripten) {
			optPlatformOSWindows = false;
			optPlatformOSLinux = true;
		};

		TPointer<ICompiler> compiler;
		if (optPlatformCompilerMSVC) {
			compiler = TMemory<CompilerMSVC>::newMemory();
		} else if (optPlatformCompilerGCC) {
			compiler = TMemory<CompilerGCC>::newMemory();
		} else {
			printf("Error: no compiler, use --platform-compiler-msvc or --platform-compiler-gcc\n");
			return 1;
		};

		compiler->is64Bit = optPlatform64bit;
		compiler->is32Bit = optPlatform32bit;
		compiler->isOSWindows = optPlatformOSWindows;
		compiler->isOSLinux = optPlatformOSLinux;
		compiler->isOSEmscripten = optPlatformOSEmscripten;

		compiler->isStatic = false;
		if (Shell::hasEnv("XYO_PLATFORM_COMPILE_STATIC")) {
			String env = Shell::getEnv("XYO_PLATFORM_COMPILE_STATIC");
			if (env == "1" || env == "ON" || env == "TRUE") {
				cppDefine.push("XYO_PLATFORM_COMPILE_STATIC");
				compiler->isStatic = true;
			};
		};

		// --- sources

		// headers are dependencies of all sources
		TDynamicArray<String> incFiles;
		for (k = 0; k < hFiles.length(); ++k) {
			incFiles.push(hFiles[k]);
		};
		for (k = 0; k < hppFiles.length(); ++k) {
			incFiles.push(hppFiles[k]);
		};

		// C and C++ sources mixed, the C++ compiler takes precedence, C sources are compiled as C++
		if ((!cSource.isEmpty()) && (!cppSource.isEmpty())) {
			printf("Info: C and C++ sources mixed, C sources are compiled as C++\n");
			for (k = 0; k < cSource.length(); ++k) {
				cppSource.push(cSource[k]);
			};
			cSource.empty();
		};

		// ---

		if (makeLibrary) {
			if ((cSource.isEmpty()) && (cppSource.isEmpty())) {
				printf("Error: no c/cpp source for library %s\n", projectName.value());
				return 1;
			};
			if (cSource.length() > 0) {
				if (!compiler->makeCToLib(
				        libName.length() ? libName : projectName,
				        outputBinPath,
				        outputLibPath,
				        tempPath,
				        (isRelease ? CompilerOptions::Release : CompilerOptions::Debug) | crtOption | CompilerOptions::StaticLibrary,
				        cppDefine,
				        incPath,
				        incFiles,
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
				        libName.length() ? libName : projectName,
				        outputBinPath,
				        outputLibPath,
				        tempPath,
				        (isRelease ? CompilerOptions::Release : CompilerOptions::Debug) | crtOption | CompilerOptions::StaticLibrary,
				        cppDefine,
				        incPath,
				        incFiles,
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
				        incFiles,
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
				        incFiles,
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
				        (isRelease ? CompilerOptions::Release : CompilerOptions::Debug) | crtOption | (compiler->isStatic ? CompilerOptions::StaticLibrary : CompilerOptions::DynamicLibrary),
				        cppDefine,
				        incPath,
				        incFiles,
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
				        (isRelease ? CompilerOptions::Release : CompilerOptions::Debug) | crtOption | (compiler->isStatic ? CompilerOptions::StaticLibrary : CompilerOptions::DynamicLibrary),
				        cppDefine,
				        incPath,
				        incFiles,
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
