// C++ Compiler Command Driver
// Copyright (c) 2020-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2020-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/CPPCompilerCommandDriver/CompilerGCC.hpp>

namespace XYO::CPPCompilerCommandDriver {

	CompilerGCC::CompilerGCC() {
		type = CompilerType::GCC;
		isOSWindows = false;
		isOSLinux = false;
		is32Bit = false;
		is64Bit = false;
	};

	String CompilerGCC::objFilename(
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
		retV << Shell::getFileName(fileName) << ".o";

		return retV;
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
		String cmd;
		String content;

		int k;
		options = filterOptions(options);
		if (!Shell::mkdirFilePath(objFile)) {
			return false;
		};

		cppFile = String::replace(cppFile, "\\", "/");
		objFile = String::replace(objFile, "\\", "/");
		String cxx = Shell::getEnv("CXX");
		if (cxx.length() == 0) {
			cxx = "gcc";
		};
		cmd = cxx;
		content = " -O1 -std=c++11 -std=gnu++11 -fpermissive";
		if (options & CompilerOptions::Release) {
			content += " -DXYO_COMPILE_RELEASE";
		};
		if (options & CompilerOptions::Debug) {
			content += " -g";
			content += " -DXYO_COMPILE_DEBUG";
		};
		if (options & CompilerOptions::CRTStatic) {
			content += " -DXYO_COMPILE_CRT_STATIC";
		};
		if (options & CompilerOptions::CRTDynamic) {
			content += " -DXYO_COMPILE_CRT_DYNAMIC";
		};
		if (options & CompilerOptions::StaticLibrary) {
			content += " -DXYO_COMPILE_STATIC_LIBRARY";
		};
		if (options & CompilerOptions::DynamicLibrary) {
			content += " -fpic";
			if (isOSLinux) {
				content += " -rdynamic";
			};
			if (options & CompilerOptions::DynamicLibraryXStatic) {
				content += " -DXYO_COMPILE_STATIC_LIBRARY";
			} else {
				content += " -DXYO_COMPILE_DYNAMIC_LIBRARY";
			};
		};
		for (k = 0; k < incPath.length(); ++k) {
			content << " -I\"" << String::replace(incPath[k], "\\", "/") << "\"";
		};
		for (k = 0; k < cppDefine.length(); ++k) {
			content << " -D\"" << cppDefine[k] << "\"";
		};
		content << " -c -o \"" << objFile << "\"";
		content << " \"" << cppFile << "\"";

		String cmdFile = String::replace(objFile, ".cpp.o", ".cpp2o");
		Shell::filePutContents(cmdFile, content);
		cmd << " @" << cmdFile;

		if (echoCmd) {
			printf("[%d/%d] %s\n", index, indexLn, cmd.value());
		};
		return (Shell::system(cmd) == 0);
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
		int k;
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

		binPath = String::replace(binPath, "\\", "/");
		libPath = String::replace(libPath, "\\", "/");
		tmpPath = String::replace(tmpPath, "\\", "/");

		if (options & CompilerOptions::StaticLibrary) {
			libNameOut = libPath + "/" + libName + ".a";
			if (!force) {
				if (!Shell::isChanged(libNameOut, objFiles)) {
					return true;
				};
			};
			Shell::remove(libNameOut);
			for (k = 0; k < objFiles.length(); ++k) {
				cmd = "ar qcs";
				cmd << " \"" << libNameOut << "\"";
				cmd << " \"" << String::replace(objFiles[k], "\\", "/") << "\"";
				if (echoCmd) {
					printf("[%d/%d] %s\n", (int)(k + 1), (int)objFiles.length(), cmd.value());
				};
				if (Shell::system(cmd) != 0) {
					Shell::remove(libNameOut);
					return false;
				};
			};
			return true;
		};

		if (options & CompilerOptions::DynamicLibrary) {
			if (isOSLinux) {
				libNameOut = binPath << "/" << libName << ".so";
				if (!version.isEmpty()) {
					libNameOut << "." << version;
				};
			};
			if (isOSWindows) {
				libNameOut = binPath << "/" << libName;
				if (!version.isEmpty()) {
					libNameOut << "-" << version;
				};
				libNameOut << ".dll";
			};
			if (!force) {
				if (!Shell::isChanged(libNameOut, objFiles)) {
					return true;
				};
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
				content << " \"" << String::replace(objFiles[k], "\\", "/") << "\"";
			};
			for (k = 0; k < libDependencyPath.length(); ++k) {
				if (isOSLinux) {
					content << " -L\"" << libDependencyPath[k] << "\"";
				};
				if (isOSWindows) {
					content << " -L\"" << String::replace(libDependencyPath[k], "\\", "/") << "\"";
				};
			};
			for (k = 0; k < libDependency.length(); ++k) {
				if (String::endsWith(libDependency[k], ".static")) {
					if (libDependency[k][0] == ':') {
						content << " -l" << libDependency[k] << ".a";
						continue;
					};
					content << " -l" << String::replace(libDependency[k], "lib", "");
					continue;
				};
				if (libDependency[k][0] == ':') {
					if (isOSLinux) {
						content << " -l" << libDependency[k] << ".so";
					};
					if (isOSWindows) {
						content << " -l" << libDependency[k] << ".dll";
					};
					continue;
				};
				content << " -l" << String::replace(libDependency[k], "lib", "");
			};
			content << " -lstdc++";
			content << " -lpthread";
			content << " -lm";
			if (isOSLinux) {
				content << " -ldl";
			};
			if (isOSWindows) {
				content << " -luser32 -lws2_32";
			};

			Shell::filePutContents(tmpPath + "/" + libName + ".o2so", content);
			String cxx = Shell::getEnv("CXX");
			if (cxx.length() == 0) {
				cxx = "gcc";
			};
			cmd = cxx + " @";
			cmd << tmpPath + "/" + libName + ".o2so";
			if (echoCmd) {
				printf("%s\n", cmd.value());
			};
			if (Shell::system(cmd) == 0) {
				if (isOSLinux) {
					return Shell::copy(libNameOut, libPath + "/" + libName + ".so");
				};
				if (isOSWindows) {
					return Shell::copy(libNameOut, libPath + "/" + libName + ".dll");
				};
			};
			return false;
		};

		if (echoCmd) {
			printf("%s\n", cmd.value());
		};
		if (Shell::system(cmd) == 0) {
			return true;
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
		if (isOSWindows) {
			options = filterOptions(options);
		};

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

		binPath = String::replace(binPath, "\\", "/");
		tmpPath = String::replace(tmpPath, "\\", "/");

		exeNameOut = binPath << "/" << exeName;
		if (isOSWindows) {
			if (!String::endsWith(exeNameOut, ".exe")) {
				exeNameOut << ".exe";
			};
		};
		if (!force) {
			if (!Shell::isChanged(exeNameOut, objFiles)) {
				return true;
			};
		};

		content << "-o \"" << exeNameOut << "\" -Wl,-rpath='$ORIGIN'";
		for (k = 0; k < objFiles.length(); ++k) {
			content << " \"" << String::replace(objFiles[k], "\\", "/") << "\"";
		};
		for (k = 0; k < libDependencyPath.length(); ++k) {
			if (isOSLinux) {
				content << " -L\"" << libDependencyPath[k] << "\"";
			};
			if (isOSWindows) {
				content << " -L\"" << String::replace(libDependencyPath[k], "\\", "/") << "\"";
			};
		};
		for (k = 0; k < libDependency.length(); ++k) {
			if (String::endsWith(libDependency[k], ".static")) {
				if (libDependency[k][0] == ':') {
					content << " -l" << libDependency[k] << ".a";
					continue;
				};
				content << " -l" << String::replace(libDependency[k], "lib", "");
				continue;
			};
			if (libDependency[k][0] == ':') {
				if (isOSLinux) {
					content << " -l" << libDependency[k] << ".so";
				};
				if (isOSWindows) {
					content << " -l" << libDependency[k] << ".dll";
				};
				continue;
			};
			content << " -l" << String::replace(libDependency[k], "lib", "");
		};

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
		Shell::filePutContents(tmpPath + "/" + exeName + ".o2elf", content);
		String cxx = Shell::getEnv("CXX");
		if (cxx.length() == 0) {
			cxx = "gcc";
		};
		cmd = cxx + " @";
		cmd << tmpPath + "/" + exeName + ".o2elf";

		if (echoCmd) {
			printf("%s\n", cmd.value());
		};
		if (Shell::system(cmd) == 0) {
			return true;
		};
		return false;
	};

	bool CompilerGCC::rcToRes(
	    String rcFile,
	    String resFile,
	    TDynamicArray<String> &rcDefine,
	    TDynamicArray<String> &incPath,
	    bool echoCmd) {
		if (isOSLinux) {
			return false;
		};
		if (isOSWindows) {
			String cmd;
			int k;
			if (!Shell::mkdirFilePath(resFile)) {
				return false;
			};

			rcFile = String::replace(rcFile, "\\", "/");
			resFile = String::replace(resFile, "\\", "/");
			cmd = "windres ";
			for (k = 0; k < incPath.length(); ++k) {
				cmd << " -I \"" << String::replace(incPath[k], "/", "\\") << "\"";
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
		return false;
	};

	bool CompilerGCC::resToObj(
	    String resFile,
	    String objFile,
	    bool echoCmd) {
		if (isOSLinux) {
			return false;
		};
		if (isOSWindows) {
			String cmd;
			if (!Shell::mkdirFilePath(objFile)) {
				return false;
			};

			resFile = String::replace(resFile, "/", "\\");
			objFile = String::replace(objFile, "/", "\\");

			cmd = "windres -J res -O coff";

			cmd << " -o \"" << objFile << "\" -i \"" << resFile << "\"";

			if (echoCmd) {
				printf("%s\n", cmd.value());
			};
			return (Shell::system(cmd) == 0);
		};

		return false;
	};

	bool CompilerGCC::makeRcToObj(
	    String rcFile,
	    String objFile,
	    TDynamicArray<String> &rcDefine,
	    TDynamicArray<String> &incPath,
	    bool echoCmd,
	    bool force) {
		if (isOSLinux) {
			return false;
		};
		if (isOSWindows) {
			bool toMake;

			String resFile = String::replace(objFile, ".o", ".res");
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

		return false;
	}

	namespace CompilerGCCWorker {

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
				CompilerGCC *super;
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

		size_t k, m;
		TDynamicArray<String> objFiles;
		TPointer<CompilerGCCWorker::CompilerWorkerCppToObj> parameter;
		TPointer<CompilerGCCWorker::CompilerWorkerBool> retVCppToObj;
		WorkerQueue compileCppToObj;
		compileCppToObj.setNumberOfThreads(numThreads);
		String projectName = libName;
		bool toMakeCppToObj;

		if (options & CompilerOptions::DynamicLibrary) {
			projectName << ".so";
		};
		if (options & CompilerOptions::StaticLibrary) {
			projectName << ".a";
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
			TWorkerQueue<CompilerGCCWorker::CompilerWorkerBool,
			             CompilerGCCWorker::CompilerWorkerCppToObj,
			             CompilerGCCWorker::compilerTransferWorkerBool,
			             CompilerGCCWorker::compilerTransferWorkerCppToObj,
			             CompilerGCCWorker::compilerWorkerProcedureCppToObj>::add(compileCppToObj, parameter);
		};

		if (!compileCppToObj.isEmpty()) {
			if (!compileCppToObj.process()) {
				return false;
			};
			for (k = 0; k < compileCppToObj.length(); ++k) {
				retVCppToObj = TStaticCast<CompilerGCCWorker::CompilerWorkerBool *>(compileCppToObj.getReturnValue(k));
				if (retVCppToObj) {
					if (!retVCppToObj->value) {
						return false;
					};
					continue;
				};
				return false;
			};
		};

		if (isOSWindows) {
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
		if (options & CompilerOptions::CRTStatic) {
			options |= CompilerOptions::StaticLibrary;
		};
		if (options & CompilerOptions::CRTDynamic) {
			options |= CompilerOptions::DynamicLibrary;
		};
		options = filterOptions(options);

		size_t k, m;
		TDynamicArray<String> objFiles;
		TPointer<CompilerGCCWorker::CompilerWorkerCppToObj> parameter;
		TPointer<CompilerGCCWorker::CompilerWorkerBool> retVCppToObj;
		WorkerQueue compileCppToObj;
		compileCppToObj.setNumberOfThreads(numThreads);
		String projectName = exeName;
		bool toMakeCppToObj;

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
			TWorkerQueue<CompilerGCCWorker::CompilerWorkerBool,
			             CompilerGCCWorker::CompilerWorkerCppToObj,
			             CompilerGCCWorker::compilerTransferWorkerBool,
			             CompilerGCCWorker::compilerTransferWorkerCppToObj,
			             CompilerGCCWorker::compilerWorkerProcedureCppToObj>::add(compileCppToObj, parameter);
		};

		if (!compileCppToObj.isEmpty()) {
			if (!compileCppToObj.process()) {
				return false;
			};
			for (k = 0; k < compileCppToObj.length(); ++k) {
				retVCppToObj = TStaticCast<CompilerGCCWorker::CompilerWorkerBool *>(compileCppToObj.getReturnValue(k));
				if (retVCppToObj) {
					if (!retVCppToObj->value) {
						return false;
					};
					continue;
				};
				return false;
			};
		};

		if (isOSWindows) {
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
		String cmd;
		String content;

		int k;
		options = filterOptions(options);
		if (!Shell::mkdirFilePath(objFile)) {
			return false;
		};

		cFile = String::replace(cFile, "\\", "/");
		objFile = String::replace(objFile, "\\", "/");
		String cc = Shell::getEnv("CC");
		if (cc.length() == 0) {
			cc = "gcc";
		};
		cmd = cc;
		content = " -O1";
		if (options & CompilerOptions::Release) {
			content += " -DXYO_COMPILE_RELEASE";
		};
		if (options & CompilerOptions::Debug) {
			content += " -g";
			content += " -DXYO_COMPILE_DEBUG";
		};
		if (options & CompilerOptions::CRTStatic) {
			content += " -DXYO_COMPILE_CRT_STATIC";
		};
		if (options & CompilerOptions::CRTDynamic) {
			content += " -DXYO_COMPILE_CRT_DYNAMIC";
		};
		if (options & CompilerOptions::StaticLibrary) {
			content += " -DXYO_COMPILE_STATIC_LIBRARY";
		};
		if (options & CompilerOptions::DynamicLibrary) {
			content += " -fpic -rdynamic";
			if (options & CompilerOptions::DynamicLibraryXStatic) {
				content += " -DXYO_COMPILE_STATIC_LIBRARY";
			} else {
				content += " -DXYO_COMPILE_DYNAMIC_LIBRARY";
			};
		};
		for (k = 0; k < incPath.length(); ++k) {
			content << " -I\"" << String::replace(incPath[k], "\\", "/") << "\"";
		};
		for (k = 0; k < cDefine.length(); ++k) {
			content << " -D\"" << cDefine[k] << "\"";
		};
		content << " -c -o \"" << objFile << "\"";
		content << " \"" << cFile << "\"";

		String cmdFile = String::replace(objFile, ".c.o", ".c2o");
		Shell::filePutContents(cmdFile, content);
		cmd << " @" << cmdFile;

		if (echoCmd) {
			printf("[%d/%d] %s\n", index, indexLn, cmd.value());
		};
		return (Shell::system(cmd) == 0);
	};

	namespace CompilerGCCWorker {

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

		size_t k, m;
		TDynamicArray<String> objFiles;
		TPointer<CompilerGCCWorker::CompilerWorkerCppToObj> parameter;
		TPointer<CompilerGCCWorker::CompilerWorkerBool> retVCToObj;
		WorkerQueue compileCToObj;
		compileCToObj.setNumberOfThreads(numThreads);
		String projectName = libName;
		bool toMakeCToObj;

		if (options & CompilerOptions::DynamicLibrary) {
			projectName << ".so";
		};
		if (options & CompilerOptions::StaticLibrary) {
			projectName << ".a";
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
			TWorkerQueue<CompilerGCCWorker::CompilerWorkerBool,
			             CompilerGCCWorker::CompilerWorkerCppToObj,
			             CompilerGCCWorker::compilerTransferWorkerBool,
			             CompilerGCCWorker::compilerTransferWorkerCppToObj,
			             CompilerGCCWorker::compilerWorkerProcedureCToObj>::add(compileCToObj, parameter);
		};

		if (!compileCToObj.isEmpty()) {
			if (!compileCToObj.process()) {
				return false;
			};
			for (k = 0; k < compileCToObj.length(); ++k) {
				retVCToObj = TStaticCast<CompilerGCCWorker::CompilerWorkerBool *>(compileCToObj.getReturnValue(k));
				if (retVCToObj) {
					if (!retVCToObj->value) {
						return false;
					};
					continue;
				};
				return false;
			};
		};

		if (isOSWindows) {
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
		if (options & CompilerOptions::CRTStatic) {
			options |= CompilerOptions::StaticLibrary;
		};
		if (options & CompilerOptions::CRTDynamic) {
			options |= CompilerOptions::DynamicLibrary;
		};
		options = filterOptions(options);

		size_t k, m;
		TDynamicArray<String> objFiles;
		TPointer<CompilerGCCWorker::CompilerWorkerCppToObj> parameter;
		TPointer<CompilerGCCWorker::CompilerWorkerBool> retVCToObj;
		WorkerQueue compileCToObj;
		compileCToObj.setNumberOfThreads(numThreads);
		String projectName = exeName;
		bool toMakeCToObj;

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
			TWorkerQueue<CompilerGCCWorker::CompilerWorkerBool,
			             CompilerGCCWorker::CompilerWorkerCppToObj,
			             CompilerGCCWorker::compilerTransferWorkerBool,
			             CompilerGCCWorker::compilerTransferWorkerCppToObj,
			             CompilerGCCWorker::compilerWorkerProcedureCToObj>::add(compileCToObj, parameter);
		};

		if (!compileCToObj.isEmpty()) {
			if (!compileCToObj.process()) {
				return false;
			};
			for (k = 0; k < compileCToObj.length(); ++k) {
				retVCToObj = TStaticCast<CompilerGCCWorker::CompilerWorkerBool *>(compileCToObj.getReturnValue(k));
				if (retVCToObj) {
					if (!retVCToObj->value) {
						return false;
					};
					continue;
				};
				return false;
			};
		};
		if (isOSWindows) {
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
