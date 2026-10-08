// C++ Compiler Command Driver
// Copyright (c) 2020-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2020-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/CPPCompilerCommandDriver/CompilerMSVC.hpp>
#include <XYO/CPPCompilerCommandDriver/CompilerMake.hpp>

namespace XYO::CPPCompilerCommandDriver {

	CompilerMSVC::CompilerMSVC() {
		type = CompilerType::MSVC;
		isOSWindows = false;
		isOSLinux = false;
		isOSEmscripten = false;
		is32Bit = false;
		is64Bit = false;
		isStatic = false;
	};

	String CompilerMSVC::objFilename(
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
		retV << Shell::getFileName(fileName) << ".obj";

		return retV;
	};

	static String compilerMSVCCommand(const char *envName) {
		String retV = Shell::getEnv(envName);
		if (retV.length() == 0) {
			retV = "cl.exe";
		};
		return retV;
	};

	static void compilerMSVCMachine(String &content, bool is32Bit, bool is64Bit) {
		if (is64Bit) {
			content << " /MACHINE:X64";
		};
		if (is32Bit) {
			content << " /MACHINE:X86";
		};
	};

	static void compilerMSVCLinkOptions(String &content, int options) {
		content << " /INCREMENTAL:NO /OPT:REF /OPT:ICF";
		if (options & CompilerOptions::Release) {
			content << " /RELEASE";
			if (options & CompilerOptions::CRTDynamic) {
				content += " /nodefaultlib:libcmt /defaultlib:msvcrt";
			};
			if (options & CompilerOptions::CRTStatic) {
				content += " /nodefaultlib:msvcrt /defaultlib:libcmt";
			};
		};
		if (options & CompilerOptions::Debug) {
			content << " /DEBUG";
			if (options & CompilerOptions::CRTDynamic) {
				content += " /nodefaultlib:libcmtd /defaultlib:msvcrtd";
			};
			if (options & CompilerOptions::CRTStatic) {
				content += " /nodefaultlib:msvcrtd /defaultlib:libcmtd";
			};
		};
	};

	static void compilerMSVCLibDependency(
	    String &content,
	    TDynamicArray<String> &objFiles,
	    TDynamicArray<String> &libDependencyPath,
	    TDynamicArray<String> &libDependency) {
		size_t k;
		for (k = 0; k < objFiles.length(); ++k) {
			content << " \"" << objFiles[k].replace("/", "\\") << "\"";
		};
		for (k = 0; k < libDependencyPath.length(); ++k) {
			content << " /LIBPATH:\"" << libDependencyPath[k] << "\"";
		};
		for (k = 0; k < libDependency.length(); ++k) {
			if (libDependency[k][0] == ':') {
				content << " " << libDependency[k].substring(1) << ".lib";
				continue;
			};
			content << " " << libDependency[k] << ".lib";
		};

		content << " user32.lib";
		content << " gdi32.lib";
		content << " ws2_32.lib";
		content << " ole32.lib";
		content << " wininet.lib";
		content << " advapi32.lib";
		content << " oleaut32.lib";
		content << " shell32.lib";
		content << " iphlpapi.lib";
	};

	// Write the response file and run the tool, skip if nothing changed
	static bool compilerMSVCRun(
	    const char *tool,
	    const String &target,
	    const String &cmdFile,
	    const String &content,
	    TDynamicArray<String> &objFiles,
	    bool echoCmd,
	    bool force) {
		if (!force) {
			if (!Shell::isChanged(target, objFiles)) {
				if (!CompilerMake::isContentChanged(cmdFile, content)) {
					return true;
				};
			};
		};

		if (!Shell::filePutContents(cmdFile, content)) {
			return false;
		};

		String cmd = tool;
		cmd << " @\"" << cmdFile << "\"";
		if (echoCmd) {
			printf("%s\n", cmd.value());
		};
		if (Shell::system(cmd) != 0) {
			// build again next time
			Shell::remove(cmdFile);
			return false;
		};
		return true;
	};

	static bool compilerMSVCToObj(
	    const String &compiler,
	    const char *language,
	    int options,
	    String srcFile,
	    String objFile,
	    TDynamicArray<String> &define,
	    TDynamicArray<String> &incPath,
	    int index,
	    int indexLn,
	    bool is32Bit,
	    bool echoCmd) {
		String cmd;
		String content;
		size_t k;

		if (!Shell::mkdirFilePath(objFile)) {
			return false;
		};

		srcFile = srcFile.replace("/", "\\");
		objFile = objFile.replace("/", "\\");

		content = " /nologo";
		content << language;

		if (options & CompilerOptions::Release) {
			content += " /DXYO_PLATFORM_COMPILE_RELEASE";
			if (options & CompilerOptions::CRTDynamic) {
				content += " /MD";
				content += " /DXYO_PLATFORM_COMPILE_CRT_DYNAMIC";
			};
			if (options & CompilerOptions::CRTStatic) {
				content += " /MT";
				content += " /DXYO_PLATFORM_COMPILE_CRT_STATIC";
			};
			content += " /O2 /Oi /Oy /Gy /Gd /EHsc /GR /c";
		};
		if (options & CompilerOptions::Debug) {
			content += " /DXYO_PLATFORM_COMPILE_DEBUG";
			if (options & CompilerOptions::CRTDynamic) {
				content += " /MDd";
				content += " /DXYO_PLATFORM_COMPILE_CRT_DYNAMIC";
			};
			if (options & CompilerOptions::CRTStatic) {
				content += " /MTd";
				content += " /DXYO_PLATFORM_COMPILE_CRT_STATIC";
			};
			content += " /Zi /EHsc /GR /c";
		};
		if (options & CompilerOptions::StaticLibrary) {
			content += " /DXYO_PLATFORM_COMPILE_STATIC_LIBRARY";
		};
		if (options & CompilerOptions::DynamicLibrary) {
			if (options & CompilerOptions::DynamicLibraryXStatic) {
				content += " /DXYO_PLATFORM_COMPILE_STATIC_LIBRARY";
			} else {
				content += " /DXYO_PLATFORM_COMPILE_DYNAMIC_LIBRARY";
			};
		};

		for (k = 0; k < incPath.length(); ++k) {
			content << " /I\"" << incPath[k].replace("/", "\\") << "\"";
		};
		for (k = 0; k < define.length(); ++k) {
			content << " /D\"" << define[k] << "\"";
		};
		if (options & CompilerOptions::Debug) {
			content << " /Fd\"" << CompilerMake::replaceSuffix(objFile, ".obj", ".pdb") << "\"";
		};

		if (is32Bit) {
			content << " /arch:SSE2";
		};
		content << " /Fo\"" << objFile << "\"";
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

	bool CompilerMSVC::cppToObj(
	    int options,
	    String cppFile,
	    String objFile,
	    TDynamicArray<String> &cppDefine,
	    TDynamicArray<String> &incPath,
	    int index,
	    int indexLn,
	    bool echoCmd) {
		// /TP - all sources are C++, also C sources mixed with C++ sources
		return compilerMSVCToObj(
		    compilerMSVCCommand("CXX"),
		    " /std:c++17 /TP",
		    filterOptions(options),
		    cppFile,
		    objFile,
		    cppDefine,
		    incPath,
		    index,
		    indexLn,
		    is32Bit,
		    echoCmd);
	};

	bool CompilerMSVC::makeObjToLib(
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

		size_t k;
		String content;
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

		binPath = binPath.replace("/", "\\");
		libPath = libPath.replace("/", "\\");
		tmpPath = tmpPath.replace("/", "\\");

		if (options & CompilerOptions::StaticLibrary) {
			libNameOut = libPath + "\\" + libName + ".lib";

			content << "/NOLOGO /OUT:\"" << libNameOut << "\"";
			compilerMSVCMachine(content, is32Bit, is64Bit);
			for (k = 0; k < objFiles.length(); ++k) {
				content << " \"" << objFiles[k].replace("/", "\\") << "\"";
			};

			return compilerMSVCRun("lib.exe", libNameOut, tmpPath + "\\" + libName + ".obj2lib", content, objFiles, echoCmd, force);
		};

		if (options & CompilerOptions::DynamicLibrary) {
			libNameOut = binPath + "\\" + libName;
			if (!version.isEmpty()) {
				libNameOut << "-" << version;
			};
			libNameOut << ".dll";

			content << "/NOLOGO /OUT:\"" << libNameOut << "\"";
			compilerMSVCMachine(content, is32Bit, is64Bit);
			if (is64Bit) {
				content << " /ENTRY:_DllMainCRTStartup";
			};
			if (is32Bit) {
				content << " /ENTRY:_DllMainCRTStartup@12";
			};
			content << " /DLL";
			compilerMSVCLinkOptions(content, options);
			if (!defFile.isEmpty()) {
				content << " /DEF:\"" << defFile << "\"";
			};
			content << " /implib:\"" << libPath << "\\" << libName << ".lib\"";
			compilerMSVCLibDependency(content, objFiles, libDependencyPath, libDependency);

			return compilerMSVCRun("link.exe", libNameOut, tmpPath + "\\" + libName + ".obj2dll", content, objFiles, echoCmd, force);
		};

		return false;
	};

	bool CompilerMSVC::makeObjToExe(
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

		String content;
		String exeNameOut;

		if (objFiles.isEmpty()) {
			return false;
		};

		if (!Shell::mkdirRecursivelyIfNotExists(binPath)) {
			return false;
		};
		if (!Shell::mkdirRecursivelyIfNotExists(tmpPath)) {
			return false;
		};

		binPath = binPath.replace("/", "\\");
		tmpPath = tmpPath.replace("/", "\\");

		exeNameOut = binPath + "\\" + exeName;
		if (!exeNameOut.endsWith(".exe")) {
			exeNameOut << ".exe";
		};

		content << "/NOLOGO /OUT:\"" << exeNameOut << "\"";
		compilerMSVCMachine(content, is32Bit, is64Bit);
		compilerMSVCLinkOptions(content, options);
		compilerMSVCLibDependency(content, objFiles, libDependencyPath, libDependency);

		return compilerMSVCRun("link.exe", exeNameOut, tmpPath + "\\" + exeName + ".obj2exe", content, objFiles, echoCmd, force);
	};

	bool CompilerMSVC::rcToRes(
	    String rcFile,
	    String resFile,
	    TDynamicArray<String> &rcDefine,
	    TDynamicArray<String> &incPath,
	    bool echoCmd) {
		String cmd;
		size_t k;
		if (!Shell::mkdirFilePath(resFile)) {
			return false;
		};

		rcFile = rcFile.replace("/", "\\");
		resFile = resFile.replace("/", "\\");
		cmd = "rc.exe /nologo";
		for (k = 0; k < incPath.length(); ++k) {
			cmd << " /i \"" << incPath[k].replace("/", "\\") << "\"";
		};
		for (k = 0; k < rcDefine.length(); ++k) {
			cmd << " /d \"" << rcDefine[k] << "\"";
		};
		cmd << " /l 409 /z \"MS Sans Serif,Helv/MS Shell Dlg\" /r /fo \"" << resFile << "\"";
		cmd << " \"" << rcFile << "\"";

		if (echoCmd) {
			printf("%s\n", cmd.value());
		};
		return (Shell::system(cmd) == 0);
	};

	bool CompilerMSVC::resToObj(
	    String resFile,
	    String objFile,
	    bool echoCmd) {
		String cmd;
		if (!Shell::mkdirFilePath(objFile)) {
			return false;
		};

		resFile = resFile.replace("/", "\\");
		objFile = objFile.replace("/", "\\");

		cmd = "cvtres.exe /NOLOGO";
		compilerMSVCMachine(cmd, is32Bit, is64Bit);
		cmd << " /OUT:\"" << objFile << "\" \"" << resFile << "\"";

		if (echoCmd) {
			printf("%s\n", cmd.value());
		};
		return (Shell::system(cmd) == 0);
	};

	bool CompilerMSVC::makeRcToObj(
	    String rcFile,
	    String objFile,
	    TDynamicArray<String> &rcDefine,
	    TDynamicArray<String> &incPath,
	    bool echoCmd,
	    bool force) {
		bool toMake;

		String resFile = CompilerMake::replaceSuffix(objFile, ".obj", ".res");
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

	bool CompilerMSVC::makeCppToLib(
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
			projectName << ".dll";
		};
		if (options & CompilerOptions::StaticLibrary) {
			projectName << ".lib";
		};

		if (!CompilerMake::compileSources(this, false, projectName, tmpPath, options, cppDefine, incPath, incFiles, cppFiles, numThreads, echoCmd, force, objFiles)) {
			return false;
		};

		if (options & CompilerOptions::DynamicLibrary) {
			if (!CompilerMake::compileResources(this, projectName, tmpPath, options, incFiles, rcDefine, incPathRC, rcFiles, echoCmd, force, objFiles)) {
				return false;
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

	bool CompilerMSVC::makeCppToExe(
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
		projectName << ".exe";

		if (!CompilerMake::compileSources(this, false, projectName, tmpPath, options, cppDefine, incPath, incFiles, cppFiles, numThreads, echoCmd, force, objFiles)) {
			return false;
		};

		if (!CompilerMake::compileResources(this, projectName, tmpPath, options, incFiles, rcDefine, incPathRC, rcFiles, echoCmd, force, objFiles)) {
			return false;
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

	bool CompilerMSVC::cToObj(
	    String cFile,
	    String objFile,
	    int options,
	    TDynamicArray<String> &cDefine,
	    TDynamicArray<String> &incPath,
	    int index,
	    int indexLn,
	    bool echoCmd) {
		return compilerMSVCToObj(
		    compilerMSVCCommand("CC"),
		    " /TC",
		    filterOptions(options),
		    cFile,
		    objFile,
		    cDefine,
		    incPath,
		    index,
		    indexLn,
		    is32Bit,
		    echoCmd);
	};

	bool CompilerMSVC::makeCToLib(
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
			projectName << ".dll";
		};
		if (options & CompilerOptions::StaticLibrary) {
			projectName << ".lib";
		};

		if (!CompilerMake::compileSources(this, true, projectName, tmpPath, options, cDefine, incPath, incFiles, cFiles, numThreads, echoCmd, force, objFiles)) {
			return false;
		};

		if (options & CompilerOptions::DynamicLibrary) {
			if (!CompilerMake::compileResources(this, projectName, tmpPath, options, incFiles, rcDefine, incPathRC, rcFiles, echoCmd, force, objFiles)) {
				return false;
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

	bool CompilerMSVC::makeCToExe(
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
		projectName << ".exe";

		if (!CompilerMake::compileSources(this, true, projectName, tmpPath, options, cDefine, incPath, incFiles, cFiles, numThreads, echoCmd, force, objFiles)) {
			return false;
		};

		if (!CompilerMake::compileResources(this, projectName, tmpPath, options, incFiles, rcDefine, incPathRC, rcFiles, echoCmd, force, objFiles)) {
			return false;
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
