// C++ Compiler Command Driver
// Copyright (c) 2020-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2020-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/CPPCompilerCommandDriver/CompilerGCC.hpp>
#include <XYO/CPPCompilerCommandDriver/CompilerMake.hpp>

namespace XYO::CPPCompilerCommandDriver {

	CompilerGCC::CompilerGCC() {
		type = CompilerType::GCC;
		isOSWindows = false;
		isOSLinux = false;
		isOSEmscripten = false;
		is32Bit = false;
		is64Bit = false;
		isStatic = false;
	};

	String CompilerGCC::objFilename(
	    const String &project,
	    const String &fileName,
	    const String &tmpPath,
	    int options,
	    int index,
	    int indexLn) {

		options = filterOptions(options);

		String retV = tmpPath;
		retV << Shell::pathSeparator << Shell::getFileName(project) << ".";
		retV << (NumberX::leftPadByDigits(index, indexLn)).value() << ".";
		retV << CompilerMake::optionsTag(options) << ".";
		retV << Shell::getFileName(fileName) << ".o";

		return retV;
	};

	static String compilerGCCCommand(const char *envName, bool isOSEmscripten) {
		String retV = Shell::getEnv(envName);
		if (retV.length() == 0) {
			retV = "gcc";
			if (isOSEmscripten) {
				retV = "emcc";
			};
		};
		return retV;
	};

	static void compilerGCCLibDependency(
	    String &content,
	    TDynamicArray<String> &libDependencyPath,
	    TDynamicArray<String> &libDependency,
	    bool isOSLinux,
	    bool isOSWindows,
	    bool isStatic) {
		size_t k;
		for (k = 0; k < libDependencyPath.length(); ++k) {
			if (isOSLinux) {
				content << " -L\"" << libDependencyPath[k] << "\"";
			};
			if (isOSWindows) {
				content << " -L\"" << libDependencyPath[k].replace("\\", "/") << "\"";
			};
		};
		for (k = 0; k < libDependency.length(); ++k) {
			if (libDependency[k][0] == ':') {
				if (libDependency[k].endsWith(".static")) {
					content << " -l" << libDependency[k] << ".a";
					continue;
				};
				if (isOSLinux) {
					if (isStatic) {
						content << " -l" << libDependency[k] << ".a";
					} else {
						content << " -l" << libDependency[k] << ".so";
					};
				};
				if (isOSWindows) {
					if (isStatic) {
						content << " -l" << libDependency[k] << ".a";
					} else {
						content << " -l" << libDependency[k] << ".dll";
					};
				};
				continue;
			};
			content << " -l" << CompilerMake::libraryName(libDependency[k]);
		};
	};

	static bool compilerGCCToObj(
	    const String &compiler,
	    const String &flags,
	    int options,
	    String srcFile,
	    String objFile,
	    TDynamicArray<String> &define,
	    TDynamicArray<String> &incPath,
	    int index,
	    int indexLn,
	    bool isOSLinux,
	    bool isOSEmscripten,
	    bool echoCmd) {
		String cmd;
		String content;
		size_t k;

		if (!Shell::mkdirFilePath(objFile)) {
			return false;
		};

		srcFile = srcFile.replace("\\", "/");
		objFile = objFile.replace("\\", "/");

		if (options & CompilerOptions::Release) {
			content = " -O2";
			content += " -DXYO_PLATFORM_COMPILE_RELEASE";
		};
		if (options & CompilerOptions::Debug) {
			content = " -O0 -g";
			content += " -DXYO_PLATFORM_COMPILE_DEBUG";
		};
		content += flags;
		if (isOSEmscripten) {
			content += " -pthread";
		};
		if (options & CompilerOptions::CRTStatic) {
			content += " -DXYO_PLATFORM_COMPILE_CRT_STATIC";
		};
		if (options & CompilerOptions::CRTDynamic) {
			content += " -DXYO_PLATFORM_COMPILE_CRT_DYNAMIC";
		};
		if (options & CompilerOptions::StaticLibrary) {
			content += " -DXYO_PLATFORM_COMPILE_STATIC_LIBRARY";
		};
		if (options & CompilerOptions::DynamicLibrary) {
			content += " -fpic";
			if (isOSLinux) {
				if (!isOSEmscripten) {
					content += " -rdynamic";
				};
			};
			if (options & CompilerOptions::DynamicLibraryXStatic) {
				content += " -DXYO_PLATFORM_COMPILE_STATIC_LIBRARY";
			} else {
				content += " -DXYO_PLATFORM_COMPILE_DYNAMIC_LIBRARY";
			};
		};
		for (k = 0; k < incPath.length(); ++k) {
			content << " -I\"" << incPath[k].replace("\\", "/") << "\"";
		};
		for (k = 0; k < define.length(); ++k) {
			content << " -D\"" << define[k] << "\"";
		};
		content << " -c -o \"" << objFile << "\"";
		content << " \"" << srcFile << "\"";

		String cmdFile = CompilerMake::commandFilename(objFile);
		if (!Shell::filePutContents(cmdFile, content)) {
			return false;
		};
		cmd = compiler;
		cmd << " @\"" << cmdFile << "\"";

		if (echoCmd) {
			printf("[%s/%d] %s\n", (NumberX::leftPadByDigits(index, indexLn)).value(), indexLn, cmd.value());
		};
		return (Shell::system(cmd) == 0);
	};

	bool CompilerGCC::cppToObj(
	    int options,
	    String cppFile,
	    String objFile,
	    TDynamicArray<String> &cppDefine,
	    TDynamicArray<String> &incPath,
	    int index,
	    int indexLn,
	    bool echoCmd) {
		String flags = " -std=gnu++17 -fpermissive";
		// C source mixed with C++ sources, compiled as C++
		if (cppFile.endsWith(".c")) {
			flags += " -x c++";
		};
		return compilerGCCToObj(
		    compilerGCCCommand("CXX", isOSEmscripten),
		    flags,
		    filterOptions(options),
		    cppFile,
		    objFile,
		    cppDefine,
		    incPath,
		    index,
		    indexLn,
		    isOSLinux,
		    isOSEmscripten,
		    echoCmd);
	};

	bool CompilerGCC::makeObjToLib(
	    String libName,
	    String binPath,
	    String libPath,
	    String tmpPath,
	    int options,
	    TDynamicArray<String> &objFiles,
	    String defFile,
	    TDynamicArray<String> &libDependencyPath,
	    TDynamicArray<String> &libDependency,
	    String version,
	    bool echoCmd,
	    bool force) {
		options = filterOptions(options);

		String cmd;
		size_t k;
		String content;
		String cmdFile;
		String libNameOut;

		if (objFiles.isEmpty()) {
			return false;
		};

		if (!Shell::mkdirRecursivelyIfNotExists(binPath)) {
			return false;
		};
		if (!Shell::mkdirRecursivelyIfNotExists(libPath)) {
			return false;
		};
		if (!Shell::mkdirRecursivelyIfNotExists(tmpPath)) {
			return false;
		};

		binPath = binPath.replace("\\", "/");
		libPath = libPath.replace("\\", "/");
		tmpPath = tmpPath.replace("\\", "/");

		if (options & CompilerOptions::StaticLibrary) {
			libNameOut = libPath + "/" + libName + ".a";

			for (k = 0; k < objFiles.length(); ++k) {
				content << "\"" << objFiles[k].replace("\\", "/") << "\"\n";
			};

			cmdFile = tmpPath + "/" + libName + ".o2a";
			if (!force) {
				if (!Shell::isChanged(libNameOut, objFiles)) {
					if (!CompilerMake::isContentChanged(cmdFile, content)) {
						return true;
					};
				};
			};

			if (!Shell::filePutContents(cmdFile, content)) {
				return false;
			};
			Shell::remove(libNameOut);

			// all object files in one call, the symbol index is built once
			cmd = "ar qcs \"";
			cmd << libNameOut << "\" @\"" << cmdFile << "\"";
			if (echoCmd) {
				printf("%s\n", cmd.value());
			};
			if (Shell::system(cmd) != 0) {
				Shell::remove(libNameOut);
				Shell::remove(cmdFile);
				return false;
			};
			return true;
		};

		if (options & CompilerOptions::DynamicLibrary) {
			String libNameCopy;
			if (isOSLinux) {
				libNameOut = binPath + "/" + libName + ".so";
				if (!version.isEmpty()) {
					libNameOut << "." << version;
				};
				libNameCopy = libPath + "/" + libName + ".so";
			} else if (isOSWindows) {
				libNameOut = binPath + "/" + libName;
				if (!version.isEmpty()) {
					libNameOut << "-" << version;
				};
				libNameOut << ".dll";
				libNameCopy = libPath + "/" + libName + ".dll";
			} else {
				return false;
			};

			content << "-shared -o \"" << libNameOut << "\" -Wl,-rpath='$ORIGIN'";
			if (!version.isEmpty()) {
				if (isOSLinux) {
					content << ",-soname," << libName << ".so." << version;
				};
				if (isOSWindows) {
					content << ",-soname," << libName << "-" << version << ".dll";
				};
			};
			for (k = 0; k < objFiles.length(); ++k) {
				content << " \"" << objFiles[k].replace("\\", "/") << "\"";
			};
			compilerGCCLibDependency(content, libDependencyPath, libDependency, isOSLinux, isOSWindows, isStatic);
			content << " -lstdc++";
			content << " -lpthread";
			content << " -lm";
			if (isOSLinux) {
				content << " -ldl";
			};
			if (isOSWindows) {
				content << " -luser32 -lws2_32";
			};

			if (isOSEmscripten) {
				content += " -s NODERAWFS=1 -pthread";
			};

			cmdFile = tmpPath + "/" + libName + ".o2so";
			if (!force) {
				if (!Shell::isChanged(libNameOut, objFiles)) {
					if (!CompilerMake::isContentChanged(cmdFile, content)) {
						return true;
					};
				};
			};

			if (!Shell::filePutContents(cmdFile, content)) {
				return false;
			};
			cmd = compilerGCCCommand("CXX", isOSEmscripten);
			cmd << " @\"" << cmdFile << "\"";
			if (echoCmd) {
				printf("%s\n", cmd.value());
			};
			if (Shell::system(cmd) != 0) {
				Shell::remove(cmdFile);
				return false;
			};
			// same file if bin and lib path are the same and there is no version
			if (libNameOut == libNameCopy) {
				return true;
			};
			return Shell::copy(libNameOut, libNameCopy);
		};

		return false;
	};

	bool CompilerGCC::makeObjToExe(
	    String exeName,
	    String binPath,
	    String tmpPath,
	    int options,
	    TDynamicArray<String> &objFiles,
	    TDynamicArray<String> &libDependencyPath,
	    TDynamicArray<String> &libDependency,
	    bool echoCmd,
	    bool force) {
		options = filterOptions(options);

		String cmd;
		String content;
		String cmdFile;
		String exeNameOut;
		bool preRunChanged = false;

		if (objFiles.isEmpty()) {
			return false;
		};

		if (!Shell::mkdirRecursivelyIfNotExists(binPath)) {
			return false;
		};
		if (!Shell::mkdirRecursivelyIfNotExists(tmpPath)) {
			return false;
		};

		binPath = binPath.replace("\\", "/");
		tmpPath = tmpPath.replace("\\", "/");

		exeNameOut = binPath + "/" + exeName;
		if (isOSWindows) {
			if (!exeNameOut.endsWith(".exe")) {
				exeNameOut << ".exe";
			};
		};

		if (isOSEmscripten) {
			content += " -s NODERAWFS=1 -pthread ";
			String exePreRUN = tmpPath + "/" + exeName + ".prerun.js";
			String exePreRUNContent = "";
			exePreRUNContent += "Module[\"preRun\"] = () => {\r\n";
			exePreRUNContent += "\tif (ENVIRONMENT_IS_NODE) {\r\n";
			exePreRUNContent += "\t\tif (typeof(ENV) !== 'undefined') {\r\n";
			exePreRUNContent << "\t\t\tENV['EMSCRIPTEN_PLATFORM'] = process.platform;\r\n";
			exePreRUNContent << "\t\t\tENV['TEMP'] = process.env.TEMP;\r\n";
			// ---
			String exeEnv = tmpPath + "/" + exeName + ".env";
			if (Shell::fileExists(exeEnv)) {
				String exeEnvContent;
				if (Shell::fileGetContentsUTF8(exeEnv, exeEnvContent, UTFStreamMode::UTF8)) {
					TDynamicArray<String> exeEnvList;
					// one name per line, LF or CRLF
					if (exeEnvContent.explode("\n", exeEnvList)) {
						String line;
						size_t k;
						for (k = 0; k < exeEnvList.length(); ++k) {
							line = exeEnvList[k].trimASCII();
							if (line.length() == 0) {
								continue;
							};
							if (line[0] == '#') {
								continue;
							};
							exePreRUNContent << "\t\t\tENV['" << line << "'] = process.env['" << line << "'];\r\n";
						};
					};
				};
			};
			// ---
			exePreRUNContent += "\t\t}\r\n";
			exePreRUNContent += "\t}\r\n";
			exePreRUNContent += "}\r\n";
			if (CompilerMake::isContentChanged(exePreRUN, exePreRUNContent)) {
				if (!Shell::filePutContentsUTF8(exePreRUN, exePreRUNContent, UTFStreamMode::UTF8)) {
					return false;
				};
				preRunChanged = true;
			};
			content << " --pre-js \"" << exePreRUN << "\" ";
		};

		content << "-o \"" << exeNameOut << "\" -Wl,-rpath='$ORIGIN'";
		for (size_t k = 0; k < objFiles.length(); ++k) {
			content << " \"" << objFiles[k].replace("\\", "/") << "\"";
		};
		compilerGCCLibDependency(content, libDependencyPath, libDependency, isOSLinux, isOSWindows, isStatic);

		if (isOSWindows) {
			if (options & CompilerOptions::CRTStatic) {
				content << " -static-libstdc++ -static-libgcc";
				content << " -Wl,-Bstatic -lstdc++ -lpthread -lm -Wl,-Bdynamic";
			} else {
				content << " -lstdc++";
				content << " -lpthread";
				content << " -lm";
			};
			content << " -luser32 -lws2_32";
		};
		if (isOSLinux) {
			content << " -lstdc++";
			content << " -lpthread";
			content << " -lm";
			content << " -ldl";
		};

		cmdFile = tmpPath + "/" + exeName + ".o2elf";
		if (!force) {
			if (!preRunChanged) {
				if (!Shell::isChanged(exeNameOut, objFiles)) {
					if (!CompilerMake::isContentChanged(cmdFile, content)) {
						return true;
					};
				};
			};
		};

		if (!Shell::filePutContents(cmdFile, content)) {
			return false;
		};
		cmd = compilerGCCCommand("CXX", isOSEmscripten);
		cmd << " @\"" << cmdFile << "\"";

		if (echoCmd) {
			printf("%s\n", cmd.value());
		};
		if (Shell::system(cmd) != 0) {
			Shell::remove(cmdFile);
			return false;
		};
		return true;
	};

	bool CompilerGCC::rcToRes(
	    String rcFile,
	    String resFile,
	    TDynamicArray<String> &rcDefine,
	    TDynamicArray<String> &incPath,
	    bool echoCmd) {
		if (!isOSWindows) {
			return false;
		};

		String cmd;
		size_t k;
		if (!Shell::mkdirFilePath(resFile)) {
			return false;
		};

		rcFile = rcFile.replace("\\", "/");
		resFile = resFile.replace("\\", "/");
		cmd = "windres ";
		for (k = 0; k < incPath.length(); ++k) {
			cmd << " -I \"" << incPath[k].replace("/", "\\") << "\"";
		};
		for (k = 0; k < rcDefine.length(); ++k) {
			cmd << " --define \"" << rcDefine[k] << "\"";
		};
		cmd << " -l 409 -J rc -O res";
		cmd << " -o \"" << resFile << "\"";
		cmd << " -i \"" << rcFile << "\"";

		if (echoCmd) {
			printf("%s\n", cmd.value());
		};
		return (Shell::system(cmd) == 0);
	};

	bool CompilerGCC::resToObj(
	    String resFile,
	    String objFile,
	    bool echoCmd) {
		if (!isOSWindows) {
			return false;
		};

		String cmd;
		if (!Shell::mkdirFilePath(objFile)) {
			return false;
		};

		resFile = resFile.replace("/", "\\");
		objFile = objFile.replace("/", "\\");

		cmd = "windres -J res -O coff";
		cmd << " -o \"" << objFile << "\" -i \"" << resFile << "\"";

		if (echoCmd) {
			printf("%s\n", cmd.value());
		};
		return (Shell::system(cmd) == 0);
	};

	bool CompilerGCC::makeRcToObj(
	    String rcFile,
	    String objFile,
	    TDynamicArray<String> &rcDefine,
	    TDynamicArray<String> &incPath,
	    bool echoCmd,
	    bool force) {
		if (!isOSWindows) {
			return false;
		};

		bool toMake;

		String resFile = CompilerMake::replaceSuffix(objFile, ".o", ".res");
		toMake = false;
		if (!Shell::fileExists(resFile)) {
			toMake = true;
		} else {
			if (Shell::compareLastWriteTime(resFile, rcFile) < 0) {
				toMake = true;
			};
		};

		if (toMake || force) {
			if (!rcToRes(rcFile, resFile, rcDefine, incPath, echoCmd)) {
				return false;
			};
		};

		toMake = false;
		if (!Shell::fileExists(objFile)) {
			toMake = true;
		} else {
			if (Shell::compareLastWriteTime(objFile, resFile) < 0) {
				toMake = true;
			};
		};

		if (toMake || force) {
			if (!resToObj(resFile, objFile, echoCmd)) {
				return false;
			};
		};

		return true;
	};

	bool CompilerGCC::makeCppToLib(
	    String libName,
	    String binPath,
	    String libPath,
	    String tmpPath,
	    int options,
	    TDynamicArray<String> &cppDefine,
	    TDynamicArray<String> &incPath,
	    TDynamicArray<String> &incFiles,
	    TDynamicArray<String> &cppFiles,
	    TDynamicArray<String> &rcDefine,
	    TDynamicArray<String> &incPathRC,
	    TDynamicArray<String> &rcFiles,
	    String defFile,
	    TDynamicArray<String> &libDependencyPath,
	    TDynamicArray<String> &libDependency,
	    String version,
	    int numThreads,
	    bool echoCmd,
	    bool force) {
		options = filterOptions(options);

		TDynamicArray<String> objFiles;
		String projectName = libName;
		if (options & CompilerOptions::DynamicLibrary) {
			projectName << ".so";
		};
		if (options & CompilerOptions::StaticLibrary) {
			projectName << ".a";
		};

		if (!CompilerMake::compileSources(this, false, projectName, tmpPath, options, cppDefine, incPath, incFiles, cppFiles, numThreads, echoCmd, force, objFiles)) {
			return false;
		};

		if (isOSWindows) {
			if (options & CompilerOptions::DynamicLibrary) {
				if (!CompilerMake::compileResources(this, projectName, tmpPath, options, incFiles, rcDefine, incPathRC, rcFiles, echoCmd, force, objFiles)) {
					return false;
				};
			};
		};

		return makeObjToLib(
		    libName,
		    binPath,
		    libPath,
		    tmpPath,
		    options,
		    objFiles,
		    defFile,
		    libDependencyPath,
		    libDependency,
		    version,
		    echoCmd,
		    force);
	};

	bool CompilerGCC::makeCppToExe(
	    String exeName,
	    String binPath,
	    String tmpPath,
	    int options,
	    TDynamicArray<String> &cppDefine,
	    TDynamicArray<String> &incPath,
	    TDynamicArray<String> &incFiles,
	    TDynamicArray<String> &cppFiles,
	    TDynamicArray<String> &rcDefine,
	    TDynamicArray<String> &incPathRC,
	    TDynamicArray<String> &rcFiles,
	    TDynamicArray<String> &libDependencyPath,
	    TDynamicArray<String> &libDependency,
	    int numThreads,
	    bool echoCmd,
	    bool force) {
		options = filterOptions(options);

		TDynamicArray<String> objFiles;
		String projectName = exeName;

		if (!CompilerMake::compileSources(this, false, projectName, tmpPath, options, cppDefine, incPath, incFiles, cppFiles, numThreads, echoCmd, force, objFiles)) {
			return false;
		};

		if (isOSWindows) {
			if (!CompilerMake::compileResources(this, projectName, tmpPath, options, incFiles, rcDefine, incPathRC, rcFiles, echoCmd, force, objFiles)) {
				return false;
			};
		};

		return makeObjToExe(
		    exeName,
		    binPath,
		    tmpPath,
		    options,
		    objFiles,
		    libDependencyPath,
		    libDependency,
		    echoCmd,
		    force);
	};

	bool CompilerGCC::cToObj(
	    String cFile,
	    String objFile,
	    int options,
	    TDynamicArray<String> &cDefine,
	    TDynamicArray<String> &incPath,
	    int index,
	    int indexLn,
	    bool echoCmd) {
		return compilerGCCToObj(
		    compilerGCCCommand("CC", isOSEmscripten),
		    "",
		    filterOptions(options),
		    cFile,
		    objFile,
		    cDefine,
		    incPath,
		    index,
		    indexLn,
		    isOSLinux,
		    isOSEmscripten,
		    echoCmd);
	};

	bool CompilerGCC::makeCToLib(
	    String libName,
	    String binPath,
	    String libPath,
	    String tmpPath,
	    int options,
	    TDynamicArray<String> &cDefine,
	    TDynamicArray<String> &incPath,
	    TDynamicArray<String> &incFiles,
	    TDynamicArray<String> &cFiles,
	    TDynamicArray<String> &rcDefine,
	    TDynamicArray<String> &incPathRC,
	    TDynamicArray<String> &rcFiles,
	    String defFile,
	    TDynamicArray<String> &libDependencyPath,
	    TDynamicArray<String> &libDependency,
	    String version,
	    int numThreads,
	    bool echoCmd,
	    bool force) {
		options = filterOptions(options);

		TDynamicArray<String> objFiles;
		String projectName = libName;
		if (options & CompilerOptions::DynamicLibrary) {
			projectName << ".so";
		};
		if (options & CompilerOptions::StaticLibrary) {
			projectName << ".a";
		};

		if (!CompilerMake::compileSources(this, true, projectName, tmpPath, options, cDefine, incPath, incFiles, cFiles, numThreads, echoCmd, force, objFiles)) {
			return false;
		};

		if (isOSWindows) {
			if (options & CompilerOptions::DynamicLibrary) {
				if (!CompilerMake::compileResources(this, projectName, tmpPath, options, incFiles, rcDefine, incPathRC, rcFiles, echoCmd, force, objFiles)) {
					return false;
				};
			};
		};

		return makeObjToLib(
		    libName,
		    binPath,
		    libPath,
		    tmpPath,
		    options,
		    objFiles,
		    defFile,
		    libDependencyPath,
		    libDependency,
		    version,
		    echoCmd,
		    force);
	};

	bool CompilerGCC::makeCToExe(
	    String exeName,
	    String binPath,
	    String tmpPath,
	    int options,
	    TDynamicArray<String> &cDefine,
	    TDynamicArray<String> &incPath,
	    TDynamicArray<String> &incFiles,
	    TDynamicArray<String> &cFiles,
	    TDynamicArray<String> &rcDefine,
	    TDynamicArray<String> &incPathRC,
	    TDynamicArray<String> &rcFiles,
	    TDynamicArray<String> &libDependencyPath,
	    TDynamicArray<String> &libDependency,
	    int numThreads,
	    bool echoCmd,
	    bool force) {
		options = filterOptions(options);

		TDynamicArray<String> objFiles;
		String projectName = exeName;

		if (!CompilerMake::compileSources(this, true, projectName, tmpPath, options, cDefine, incPath, incFiles, cFiles, numThreads, echoCmd, force, objFiles)) {
			return false;
		};

		if (isOSWindows) {
			if (!CompilerMake::compileResources(this, projectName, tmpPath, options, incFiles, rcDefine, incPathRC, rcFiles, echoCmd, force, objFiles)) {
				return false;
			};
		};

		return makeObjToExe(
		    exeName,
		    binPath,
		    tmpPath,
		    options,
		    objFiles,
		    libDependencyPath,
		    libDependency,
		    echoCmd,
		    force);
	};

};
