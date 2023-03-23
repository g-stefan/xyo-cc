// C++ Compiler Command Driver
// Copyright (c) 2020-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2020-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/CPPCompilerCommandDriver/CompilerMSVC.hpp>

namespace XYO::CPPCompilerCommandDriver {

	CompilerMSVC::CompilerMSVC() {
		type = CompilerType::MSVC;
		isOSWindows = false;
		isOSLinux = false;
		is32Bit = false;
		is64Bit = false;
	};

	String CompilerMSVC::objFilename(
	    const String &project,
	    const String &fileName,
	    const String &tmpPath,
	    int options,
	    int index,
	    int indexLn) {

		int digits = 0;
		do {
			indexLn /= 10;
			digits++;
		} while (indexLn != 0);
		char strIndex[32];
		char strProcess[32];
		strProcess[0] = '%';
		strProcess[1] = '0';
		sprintf(&strProcess[2], "%d", digits);
		strcat(strProcess, "d");
		sprintf(strIndex, strProcess, index);

		options = filterOptions(options);

		String strOptions;
		if (options & CompilerOptions::Release) {
			strOptions += "R";
		};
		if (options & CompilerOptions::Debug) {
			strOptions += "D";
		};
		if (options & CompilerOptions::CRTStatic) {
			strOptions += "S";
		};
		if (options & CompilerOptions::CRTDynamic) {
			strOptions += "D";
		};
		if (options & CompilerOptions::StaticLibrary) {
			strOptions += "S";
		};
		if (options & CompilerOptions::DynamicLibrary) {
			strOptions += "D";
		};

		String retV = tmpPath;
		retV << Shell::pathSeparator << Shell::getFileName(project) << ".";
		retV << strIndex << ".";
		retV << strOptions << ".";
		retV << Shell::getFileName(fileName) << ".obj";

		return retV;
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
		String cmd;
		String content;

		int k;
		options = filterOptions(options);
		if (!Shell::mkdirFilePath(objFile)) {
			return false;
		};

		cppFile = StringX::replace(cppFile, "/", "\\");
		objFile = StringX::replace(objFile, "/", "\\");

		String cxx = Shell::getEnv("CXX");
		if (cxx.length() == 0) {
			cxx = "cl.exe";
		};

		cmd = cxx;
		content = " /nologo /std:c++17";

		if (options & CompilerOptions::Release) {
			content += " /DXYO_COMPILE_RELEASE";
			if (options & CompilerOptions::CRTDynamic) {
				content += " /MD";
				content += " /DXYO_COMPILE_CRT_DYNAMIC";
			};
			if (options & CompilerOptions::CRTStatic) {
				content += " /MT";
				content += " /DXYO_COMPILE_CRT_STATIC";
			};
			content += " /O2 /Oi /Oy /Gy /Gd /EHsc /GR /TP /c";
		};
		if (options & CompilerOptions::Debug) {
			content += " /DXYO_COMPILE_DEBUG";
			if (options & CompilerOptions::CRTDynamic) {
				content += " /MDd";
				content += " /DXYO_COMPILE_CRT_DYNAMIC";
			};
			if (options & CompilerOptions::CRTStatic) {
				content += " /MTd";
				content += " /DXYO_COMPILE_CRT_STATIC";
			};
			content += " /Zi /EHsc /GR /TP /c";
		};
		if (options & CompilerOptions::StaticLibrary) {
			content += " /DXYO_COMPILE_STATIC_LIBRARY";
		};
		if (options & CompilerOptions::DynamicLibrary) {
			if (options & CompilerOptions::DynamicLibraryXStatic) {
				content += " /DXYO_COMPILE_STATIC_LIBRARY";
			} else {
				content += " /DXYO_COMPILE_DYNAMIC_LIBRARY";
			};
		};

		for (k = 0; k < incPath.length(); ++k) {
			content << " /I\"" << StringX::replace(incPath[k], "/", "\\") << "\"";
		};
		for (k = 0; k < cppDefine.length(); ++k) {
			content << " /D\"" << cppDefine[k] << "\"";
		};
		if (options & CompilerOptions::Debug) {
			content << " /Fd\"" << StringX::replace(objFile, ".obj", ".pdb") << "\"";
		};

		if (is32Bit) {
			content << " /arch:SSE2";
		};
		content << " /Fo\"" << objFile << "\"";
		content << " \"" << cppFile << "\"";

		String cmdFile = StringX::replace(objFile, ".cpp.obj", ".cpp2obj");
		Shell::filePutContents(cmdFile, content);
		cmd << " @" << cmdFile;

		if (echoCmd) {
			printf("[%d/%d] %s\n", index, indexLn, cmd.value());
		};
		return (Shell::system(cmd) == 0);
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

		String cmd;
		int k;
		String content;
		bool toMakeObjToLib;
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

		binPath = StringX::replace(binPath, "/", "\\");
		libPath = StringX::replace(libPath, "/", "\\");
		tmpPath = StringX::replace(tmpPath, "/", "\\");

		if (options & CompilerOptions::StaticLibrary) {
			libNameOut = libPath << "\\" << libName << ".lib";
			if (!force) {
				if (!Shell::isChanged(libNameOut, objFiles)) {
					return true;
				};
			};

			if (is64Bit) {
				content << "/NOLOGO /OUT:\"" << libNameOut << "\" /MACHINE:X64";
			};
			if (is32Bit) {
				content << "/NOLOGO /OUT:\"" << libNameOut << "\" /MACHINE:X86";
			};

			for (k = 0; k < objFiles.length(); ++k) {
				content << " \"" << StringX::replace(objFiles[k], "/", "\\") << "\"";
			};

			Shell::filePutContents(tmpPath + "\\" + libName + ".obj2lib", content);
			cmd = "lib.exe @";
			cmd << tmpPath + "\\" + libName + ".obj2lib";
		};

		if (options & CompilerOptions::DynamicLibrary) {
			libNameOut = binPath << "\\" << libName;
			if (!version.isEmpty()) {
				libNameOut << "-" << version;
			};
			libNameOut << ".dll";
			if (!force) {
				if (!Shell::isChanged(libNameOut, objFiles)) {
					return true;
				};
			};

			if (is64Bit) {
				content << "/NOLOGO /OUT:\"" << libNameOut << "\" /MACHINE:X64";
				content << " /ENTRY:_DllMainCRTStartup";
			};
			if (is32Bit) {
				content << "/NOLOGO /OUT:\"" << libNameOut << "\" /MACHINE:X86";
				content << " /ENTRY:_DllMainCRTStartup@12";
			};
			content << " /DLL /INCREMENTAL:NO /OPT:REF /OPT:ICF";
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
			if (!defFile.isEmpty()) {
				content << " /DEF:\"" << defFile << "\"";
			};
			content << " /implib:\"" << libPath << "\\" << libName << ".lib\"";
			for (k = 0; k < objFiles.length(); ++k) {
				content << " \"" << StringX::replace(objFiles[k], "/", "\\") << "\"";
			};
			for (k = 0; k < libDependencyPath.length(); ++k) {
				content << " /LIBPATH:\"" << libDependencyPath[k] << "\"";
			};
			for (k = 0; k < libDependency.length(); ++k) {
				if (libDependency[k][0] == ':') {
					content << " " << StringX::substring(libDependency[k], 1) << ".lib";
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
			content << " ole32.lib";
			content << " oleaut32.lib";
			content << " shell32.lib";
			content << " iphlpapi.lib";

			Shell::filePutContents(tmpPath + "\\" + libName + ".obj2dll", content);
			cmd = "link.exe @";
			cmd << tmpPath + "\\" + libName + ".obj2dll";
		};
		if (echoCmd) {
			printf("%s\n", cmd.value());
		};
		return (Shell::system(cmd) == 0);
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

		String cmd;
		int k;
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

		binPath = StringX::replace(binPath, "/", "\\");
		tmpPath = StringX::replace(tmpPath, "/", "\\");

		exeNameOut = binPath << "\\" << exeName << ".exe";
		if (!force) {
			if (!Shell::isChanged(exeNameOut, objFiles)) {
				return true;
			};
		};

		if (is64Bit) {
			content << "/NOLOGO /OUT:\"" << exeNameOut << "\" /MACHINE:X64";
		};
		if (is32Bit) {
			content << "/NOLOGO /OUT:\"" << exeNameOut << "\" /MACHINE:X86";
		};
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
		for (k = 0; k < objFiles.length(); ++k) {
			content << " \"" << StringX::replace(objFiles[k], "/", "\\") << "\"";
		};
		for (k = 0; k < libDependencyPath.length(); ++k) {
			content << " /LIBPATH:\"" << libDependencyPath[k] << "\"";
		};
		for (k = 0; k < libDependency.length(); ++k) {
			if (libDependency[k][0] == ':') {
				content << " " << StringX::substring(libDependency[k], 1) << ".lib";
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
		content << " ole32.lib";
		content << " oleaut32.lib";
		content << " shell32.lib";
		content << " iphlpapi.lib";

		Shell::filePutContents(tmpPath + "\\" + exeName + ".obj2exe", content);
		cmd = "link.exe @";
		cmd << tmpPath + "\\" + exeName + ".obj2exe";

		if (echoCmd) {
			printf("%s\n", cmd.value());
		};
		return (Shell::system(cmd) == 0);
	};

	bool CompilerMSVC::rcToRes(
	    String rcFile,
	    String resFile,
	    TDynamicArray<String> &rcDefine,
	    TDynamicArray<String> &incPath,
	    bool echoCmd) {
		String cmd;
		int k;
		if (!Shell::mkdirFilePath(resFile)) {
			return false;
		};

		rcFile = StringX::replace(rcFile, "/", "\\");
		resFile = StringX::replace(resFile, "/", "\\");
		cmd = "rc.exe /nologo";
		for (k = 0; k < incPath.length(); ++k) {
			cmd << " /i \"" << StringX::replace(incPath[k], "/", "\\") << "\"";
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

		resFile = StringX::replace(resFile, "/", "\\");
		objFile = StringX::replace(objFile, "/", "\\");

		cmd = "cvtres.exe /NOLOGO";

		if (is64Bit) {
			cmd << " /MACHINE:X64";
		};
		if (is32Bit) {
			cmd << " /MACHINE:X86";
		};
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

		String resFile = StringX::replace(objFile, ".obj", ".res");
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

	namespace CompilerMSVCWorker {

		class CompilerWorkerBool : public Object {
			public:
				bool value;
		};

		TPointer<CompilerWorkerBool> compilerTransferWorkerBool(CompilerWorkerBool &value) {
			TPointer<CompilerWorkerBool> retV;
			retV.newMemory();
			retV->value = value.value;
			return retV;
		};

		class CompilerWorkerCppToObj : public Object {
			public:
				String cppFile;
				String objFile;
				int options;
				TDynamicArray<String> incPath;
				TDynamicArray<String> cppDefine;
				int index;
				int indexLn;
				bool echoCmd;
				CompilerMSVC *super;
		};

		TPointer<CompilerWorkerCppToObj> compilerTransferWorkerCppToObj(CompilerWorkerCppToObj &value) {
			TPointer<CompilerWorkerCppToObj> retV;
			retV.newMemory();
			retV->cppFile = value.cppFile.value();
			retV->objFile = value.objFile.value();
			retV->options = value.options;
			size_t k;
			TDynamicArray<String> *source;
			TDynamicArray<String> *target;

			source = &value.incPath;
			target = &retV->incPath;
			for (k = 0; k < source->length(); ++k) {
				(target->index(k)) = (source->index(k)).value();
			};

			source = &value.cppDefine;
			target = &retV->cppDefine;
			for (k = 0; k < source->length(); ++k) {
				(target->index(k)) = (source->index(k)).value();
			};

			retV->index = value.index;
			retV->indexLn = value.indexLn;
			retV->echoCmd = value.echoCmd;
			retV->super = value.super;
			return retV;
		};

		TPointer<CompilerWorkerBool> compilerWorkerProcedureCppToObj(CompilerWorkerCppToObj *parameter, TAtomic<bool> &requestToTerminate) {
			TPointer<CompilerWorkerBool> retV;
			retV.newMemory();
			if (parameter) {
				retV->value = parameter->super->cppToObj(
				    parameter->options,
				    parameter->cppFile,
				    parameter->objFile,
				    parameter->cppDefine,
				    parameter->incPath,
				    parameter->index,
				    parameter->indexLn,
				    parameter->echoCmd);
			};
			return retV;
		};

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

		size_t k, m;
		TDynamicArray<String> objFiles;
		TPointer<CompilerMSVCWorker::CompilerWorkerCppToObj> parameter;
		TPointer<CompilerMSVCWorker::CompilerWorkerBool> retVCppToObj;
		WorkerQueue compileCppToObj;
		compileCppToObj.setNumberOfThreads(numThreads);
		String projectName = libName;
		bool toMakeCppToObj;

		if (options & CompilerOptions::DynamicLibrary) {
			projectName << ".dll";
		};
		if (options & CompilerOptions::StaticLibrary) {
			projectName << ".lib";
		};

		TDynamicArray<FileTime> cppFilesTime;
		TDynamicArray<FileTime> incFilesTime;
		TDynamicArray<FileTime> objFilesTime;

		for (k = 0; k < incFiles.length(); ++k) {
			incFilesTime[k].getLastWriteTime(incFiles[k]);
		};

		for (k = 0; k < cppFiles.length(); ++k) {
			objFiles[k] = objFilename(projectName, cppFiles[k], tmpPath, options, (k + 1), cppFiles.length());
			cppFilesTime[k].getLastWriteTime(cppFiles[k]);
			objFilesTime[k].getLastWriteTime(objFiles[k]);
		};

		for (k = 0; k < cppFiles.length(); ++k) {
			toMakeCppToObj = false;
			if (cppFilesTime[k].isChanged(incFilesTime)) {
				Shell::touchIfExists(cppFiles[k]);
				toMakeCppToObj = true;
			};
			if (!Shell::fileExists(objFiles[k])) {
				toMakeCppToObj = true;
			} else {
				if (objFilesTime[k].compare(cppFilesTime[k]) < 0) {
					toMakeCppToObj = true;
				};
			};

			if (!force) {
				if (!toMakeCppToObj) {
					continue;
				};
			};

			parameter.newMemory();
			parameter->super = this;
			parameter->cppFile = cppFiles[k];
			parameter->objFile = objFiles[k];
			parameter->options = options;
			for (m = 0; m < incPath.length(); ++m) {
				parameter->incPath[m] = incPath[m];
			};
			for (m = 0; m < cppDefine.length(); ++m) {
				parameter->cppDefine[m] = cppDefine[m];
			};
			parameter->index = (k + 1);
			parameter->indexLn = cppFiles.length();
			parameter->echoCmd = echoCmd;
			TWorkerQueue<CompilerMSVCWorker::CompilerWorkerBool,
			             CompilerMSVCWorker::CompilerWorkerCppToObj,
			             CompilerMSVCWorker::compilerTransferWorkerBool,
			             CompilerMSVCWorker::compilerTransferWorkerCppToObj,
			             CompilerMSVCWorker::compilerWorkerProcedureCppToObj>::add(compileCppToObj, parameter);
		};

		if (!compileCppToObj.isEmpty()) {
			if (!compileCppToObj.process()) {
				return false;
			};
			for (k = 0; k < compileCppToObj.length(); ++k) {
				retVCppToObj = TStaticCast<CompilerMSVCWorker::CompilerWorkerBool *>(compileCppToObj.getReturnValue(k));
				if (retVCppToObj) {
					if (!retVCppToObj->value) {
						return false;
					};
					continue;
				};
				return false;
			};
		};

		if (options & CompilerOptions::DynamicLibrary) {
			String resObj;
			for (k = 0; k < rcFiles.length(); ++k) {

				if (Shell::isChanged(rcFiles[k], incFiles)) {
					Shell::touchIfExists(rcFiles[k]);
				};

				resObj = objFilename(projectName, rcFiles[k], tmpPath, options, (k + 1), rcFiles.length());

				if (!makeRcToObj(
				        rcFiles[k],
				        resObj,
				        rcDefine,
				        incPathRC,
				        echoCmd,
				        force)) {
					return false;
				};

				objFiles.push(resObj);
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
		if (options & CompilerOptions::CRTStatic) {
			options |= CompilerOptions::StaticLibrary;
		};
		if (options & CompilerOptions::CRTDynamic) {
			options |= CompilerOptions::DynamicLibrary;
		};
		options = filterOptions(options);

		size_t k, m;
		TDynamicArray<String> objFiles;
		TPointer<CompilerMSVCWorker::CompilerWorkerCppToObj> parameter;
		TPointer<CompilerMSVCWorker::CompilerWorkerBool> retVCppToObj;
		WorkerQueue compileCppToObj;
		compileCppToObj.setNumberOfThreads(numThreads);
		String projectName = exeName;
		bool toMakeCppToObj;

		projectName << ".exe";

		TDynamicArray<FileTime> cppFilesTime;
		TDynamicArray<FileTime> incFilesTime;
		TDynamicArray<FileTime> objFilesTime;

		for (k = 0; k < incFiles.length(); ++k) {
			incFilesTime[k].getLastWriteTime(incFiles[k]);
		};

		for (k = 0; k < cppFiles.length(); ++k) {
			objFiles[k] = objFilename(projectName, cppFiles[k], tmpPath, options, (k + 1), cppFiles.length());
			cppFilesTime[k].getLastWriteTime(cppFiles[k]);
			objFilesTime[k].getLastWriteTime(objFiles[k]);
		};

		for (k = 0; k < cppFiles.length(); ++k) {
			toMakeCppToObj = false;
			if (cppFilesTime[k].isChanged(incFilesTime)) {
				Shell::touchIfExists(cppFiles[k]);
				toMakeCppToObj = true;
			};
			if (!Shell::fileExists(objFiles[k])) {
				toMakeCppToObj = true;
			} else {
				if (objFilesTime[k].compare(cppFilesTime[k]) < 0) {
					toMakeCppToObj = true;
				};
			};

			if (!force) {
				if (!toMakeCppToObj) {
					continue;
				};
			};

			parameter.newMemory();
			parameter->super = this;
			parameter->cppFile = cppFiles[k];
			parameter->objFile = objFiles[k];
			parameter->options = options;
			for (m = 0; m < incPath.length(); ++m) {
				parameter->incPath[m] = incPath[m];
			};
			for (m = 0; m < cppDefine.length(); ++m) {
				parameter->cppDefine[m] = cppDefine[m];
			};
			parameter->index = (k + 1);
			parameter->indexLn = cppFiles.length();
			parameter->echoCmd = echoCmd;
			TWorkerQueue<CompilerMSVCWorker::CompilerWorkerBool,
			             CompilerMSVCWorker::CompilerWorkerCppToObj,
			             CompilerMSVCWorker::compilerTransferWorkerBool,
			             CompilerMSVCWorker::compilerTransferWorkerCppToObj,
			             CompilerMSVCWorker::compilerWorkerProcedureCppToObj>::add(compileCppToObj, parameter);
		};

		if (!compileCppToObj.isEmpty()) {
			if (!compileCppToObj.process()) {
				return false;
			};
			for (k = 0; k < compileCppToObj.length(); ++k) {
				retVCppToObj = TStaticCast<CompilerMSVCWorker::CompilerWorkerBool *>(compileCppToObj.getReturnValue(k));
				if (retVCppToObj) {
					if (!retVCppToObj->value) {
						return false;
					};
					continue;
				};
				return false;
			};
		};

		String resObj;
		for (k = 0; k < rcFiles.length(); ++k) {

			if (Shell::isChanged(rcFiles[k], incFiles)) {
				Shell::touchIfExists(rcFiles[k]);
			};

			resObj = objFilename(projectName, rcFiles[k], tmpPath, options, (k + 1), rcFiles.length());

			if (!makeRcToObj(
			        rcFiles[k],
			        resObj,
			        rcDefine,
			        incPathRC,
			        echoCmd,
			        force)) {
				return false;
			};

			objFiles.push(resObj);
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
		String cmd;
		String content;

		int k;
		options = filterOptions(options);
		if (!Shell::mkdirFilePath(objFile)) {
			return false;
		};

		cFile = StringX::replace(cFile, "/", "\\");
		objFile = StringX::replace(objFile, "/", "\\");

		String cc = Shell::getEnv("CC");
		if (cc.length() == 0) {
			cc = "cl.exe";
		};

		cmd = cc;
		content = " /nologo";

		if (options & CompilerOptions::Release) {
			content += " /DXYO_COMPILE_RELEASE";
			if (options & CompilerOptions::CRTDynamic) {
				content += " /MD";
				content += " /DXYO_COMPILE_CRT_DYNAMIC";
			};
			if (options & CompilerOptions::CRTStatic) {
				content += " /MT";
				content += " /DXYO_COMPILE_CRT_STATIC";
			};
			content += " /O2 /Oi /Oy /Gy /Gd /EHsc /GR /TC /c";
		};
		if (options & CompilerOptions::Debug) {
			content += " /DXYO_COMPILE_DEBUG";
			if (options & CompilerOptions::CRTDynamic) {
				content += " /MDd";
				content += " /DXYO_COMPILE_CRT_DYNAMIC";
			};
			if (options & CompilerOptions::CRTStatic) {
				content += " /MTd";
				content += " /DXYO_COMPILE_CRT_STATIC";
			};
			content += " /Zi /EHsc /GR /TC /c";
		};
		if (options & CompilerOptions::StaticLibrary) {
			content += " /DXYO_COMPILE_STATIC_LIBRARY";
		};
		if (options & CompilerOptions::DynamicLibrary) {
			if (options & CompilerOptions::DynamicLibraryXStatic) {
				content += " /DXYO_COMPILE_STATIC_LIBRARY";
			} else {
				content += " /DXYO_COMPILE_DYNAMIC_LIBRARY";
			};
		};

		for (k = 0; k < incPath.length(); ++k) {
			content << " /I\"" << StringX::replace(incPath[k], "/", "\\") << "\"";
		};
		for (k = 0; k < cDefine.length(); ++k) {
			content << " /D\"" << cDefine[k] << "\"";
		};
		if (options & CompilerOptions::Debug) {
			content << " /Fd\"" << StringX::replace(objFile, ".obj", ".pdb") << "\"";
		};
		if (is32Bit) {
			content << " /arch:SSE2";
		};
		content << " /Fo\"" << objFile << "\"";
		content << " \"" << cFile << "\"";

		String cmdFile = StringX::replace(objFile, ".c.obj", ".c2obj");
		Shell::filePutContents(cmdFile, content);
		cmd << " @" << cmdFile;

		if (echoCmd) {
			printf("[%d/%d] %s\n", index, indexLn, cmd.value());
		};
		return (Shell::system(cmd) == 0);
	};

	namespace CompilerMSVCWorker {

		TPointer<CompilerWorkerBool> compilerWorkerProcedureCToObj(CompilerWorkerCppToObj *parameter, TAtomic<bool> &requestToTerminate) {
			TPointer<CompilerWorkerBool> retV;
			retV.newMemory();
			if (parameter) {
				retV->value = parameter->super->cToObj(
				    parameter->cppFile,
				    parameter->objFile,
				    parameter->options,
				    parameter->cppDefine,
				    parameter->incPath,
				    parameter->index,
				    parameter->indexLn,
				    parameter->echoCmd);
			};
			return retV;
		};

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

		size_t k, m;
		TDynamicArray<String> objFiles;
		TPointer<CompilerMSVCWorker::CompilerWorkerCppToObj> parameter;
		TPointer<CompilerMSVCWorker::CompilerWorkerBool> retVCToObj;
		WorkerQueue compileCToObj;
		compileCToObj.setNumberOfThreads(numThreads);
		String projectName = libName;
		bool toMakeCToObj;

		if (options & CompilerOptions::DynamicLibrary) {
			projectName << ".dll";
		};
		if (options & CompilerOptions::StaticLibrary) {
			projectName << ".lib";
		};

		TDynamicArray<FileTime> cFilesTime;
		TDynamicArray<FileTime> incFilesTime;
		TDynamicArray<FileTime> objFilesTime;

		for (k = 0; k < incFiles.length(); ++k) {
			incFilesTime[k].getLastWriteTime(incFiles[k]);
		};

		for (k = 0; k < cFiles.length(); ++k) {
			objFiles[k] = objFilename(projectName, cFiles[k], tmpPath, options, (k + 1), cFiles.length());
			cFilesTime[k].getLastWriteTime(cFiles[k]);
			objFilesTime[k].getLastWriteTime(objFiles[k]);
		};

		for (k = 0; k < cFiles.length(); ++k) {
			toMakeCToObj = false;
			if (cFilesTime[k].isChanged(incFilesTime)) {
				Shell::touchIfExists(cFiles[k]);
				toMakeCToObj = true;
			};
			if (!Shell::fileExists(objFiles[k])) {
				toMakeCToObj = true;
			} else {
				if (objFilesTime[k].compare(cFilesTime[k]) < 0) {
					toMakeCToObj = true;
				};
			};

			if (!force) {
				if (!toMakeCToObj) {
					continue;
				};
			};

			parameter.newMemory();
			parameter->super = this;
			parameter->cppFile = cFiles[k];
			parameter->objFile = objFiles[k];
			parameter->options = options;
			for (m = 0; m < incPath.length(); ++m) {
				parameter->incPath[m] = incPath[m];
			};
			for (m = 0; m < cDefine.length(); ++m) {
				parameter->cppDefine[m] = cDefine[m];
			};
			parameter->index = (k + 1);
			parameter->indexLn = cFiles.length();
			parameter->echoCmd = echoCmd;
			TWorkerQueue<CompilerMSVCWorker::CompilerWorkerBool,
			             CompilerMSVCWorker::CompilerWorkerCppToObj,
			             CompilerMSVCWorker::compilerTransferWorkerBool,
			             CompilerMSVCWorker::compilerTransferWorkerCppToObj,
			             CompilerMSVCWorker::compilerWorkerProcedureCToObj>::add(compileCToObj, parameter);
		};

		if (!compileCToObj.isEmpty()) {
			if (!compileCToObj.process()) {
				return false;
			};
			for (k = 0; k < compileCToObj.length(); ++k) {
				retVCToObj = TStaticCast<CompilerMSVCWorker::CompilerWorkerBool *>(compileCToObj.getReturnValue(k));
				if (retVCToObj) {
					if (!retVCToObj->value) {
						return false;
					};
					continue;
				};
				return false;
			};
		};

		if (options & CompilerOptions::DynamicLibrary) {
			String resObj;
			for (k = 0; k < rcFiles.length(); ++k) {

				if (Shell::isChanged(rcFiles[k], incFiles)) {
					Shell::touchIfExists(rcFiles[k]);
				};

				resObj = objFilename(projectName, rcFiles[k], tmpPath, options, (k + 1), rcFiles.length());

				if (!makeRcToObj(
				        rcFiles[k],
				        resObj,
				        rcDefine,
				        incPathRC,
				        echoCmd,
				        force)) {
					return false;
				};

				objFiles.push(resObj);
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
		if (options & CompilerOptions::CRTStatic) {
			options |= CompilerOptions::StaticLibrary;
		};
		if (options & CompilerOptions::CRTDynamic) {
			options |= CompilerOptions::DynamicLibrary;
		};
		options = filterOptions(options);

		size_t k, m;
		TDynamicArray<String> objFiles;
		TPointer<CompilerMSVCWorker::CompilerWorkerCppToObj> parameter;
		TPointer<CompilerMSVCWorker::CompilerWorkerBool> retVCToObj;
		WorkerQueue compileCToObj;
		compileCToObj.setNumberOfThreads(numThreads);
		String projectName = exeName;
		bool toMakeCToObj;

		projectName << ".exe";

		TDynamicArray<FileTime> cFilesTime;
		TDynamicArray<FileTime> incFilesTime;
		TDynamicArray<FileTime> objFilesTime;

		for (k = 0; k < incFiles.length(); ++k) {
			incFilesTime[k].getLastWriteTime(incFiles[k]);
		};

		for (k = 0; k < cFiles.length(); ++k) {
			objFiles[k] = objFilename(projectName, cFiles[k], tmpPath, options, (k + 1), cFiles.length());
			cFilesTime[k].getLastWriteTime(cFiles[k]);
			objFilesTime[k].getLastWriteTime(objFiles[k]);
		};

		for (k = 0; k < cFiles.length(); ++k) {
			toMakeCToObj = false;
			if (cFilesTime[k].isChanged(incFilesTime)) {
				Shell::touchIfExists(cFiles[k]);
				toMakeCToObj = true;
			};
			if (!Shell::fileExists(objFiles[k])) {
				toMakeCToObj = true;
			} else {
				if (objFilesTime[k].compare(cFilesTime[k]) < 0) {
					toMakeCToObj = true;
				};
			};

			if (!force) {
				if (!toMakeCToObj) {
					continue;
				};
			};

			parameter.newMemory();
			parameter->super = this;
			parameter->cppFile = cFiles[k];
			parameter->objFile = objFiles[k];
			parameter->options = options;
			for (m = 0; m < incPath.length(); ++m) {
				parameter->incPath[m] = incPath[m];
			};
			for (m = 0; m < cDefine.length(); ++m) {
				parameter->cppDefine[m] = cDefine[m];
			};
			parameter->index = (k + 1);
			parameter->indexLn = cFiles.length();
			parameter->echoCmd = echoCmd;
			TWorkerQueue<CompilerMSVCWorker::CompilerWorkerBool,
			             CompilerMSVCWorker::CompilerWorkerCppToObj,
			             CompilerMSVCWorker::compilerTransferWorkerBool,
			             CompilerMSVCWorker::compilerTransferWorkerCppToObj,
			             CompilerMSVCWorker::compilerWorkerProcedureCToObj>::add(compileCToObj, parameter);
		};

		if (!compileCToObj.isEmpty()) {
			if (!compileCToObj.process()) {
				return false;
			};
			for (k = 0; k < compileCToObj.length(); ++k) {
				retVCToObj = TStaticCast<CompilerMSVCWorker::CompilerWorkerBool *>(compileCToObj.getReturnValue(k));
				if (retVCToObj) {
					if (!retVCToObj->value) {
						return false;
					};
					continue;
				};
				return false;
			};
		};

		String resObj;
		for (k = 0; k < rcFiles.length(); ++k) {

			if (Shell::isChanged(rcFiles[k], incFiles)) {
				Shell::touchIfExists(rcFiles[k]);
			};

			resObj = objFilename(projectName, rcFiles[k], tmpPath, options, (k + 1), rcFiles.length());

			if (!makeRcToObj(
			        rcFiles[k],
			        resObj,
			        rcDefine,
			        incPathRC,
			        echoCmd,
			        force)) {
				return false;
			};

			objFiles.push(resObj);
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
