// C++ Compiler Command Driver
// Copyright (c) 2020-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2020-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/CPPCompilerCommandDriver/CompilerMake.hpp>

namespace XYO::CPPCompilerCommandDriver::CompilerMake {

	String optionsTag(int options) {
		String retV;
		if (options & CompilerOptions::Release) {
			retV += "R";
		};
		if (options & CompilerOptions::Debug) {
			retV += "D";
		};
		if (options & CompilerOptions::CRTStatic) {
			retV += "S";
		};
		if (options & CompilerOptions::CRTDynamic) {
			retV += "D";
		};
		if (options & CompilerOptions::StaticLibrary) {
			retV += "S";
		};
		if (options & CompilerOptions::DynamicLibrary) {
			retV += "D";
		};
		return retV;
	};

	String replaceSuffix(const String &value, const String &suffix, const String &newSuffix) {
		if (!value.endsWith(suffix)) {
			return value;
		};
		return value.substring(0, value.length() - suffix.length()) + newSuffix;
	};

	String commandFilename(const String &objFile) {
		size_t k = objFile.length();
		while (k > 0) {
			--k;
			if (objFile[(int)k] == '.') {
				return objFile.substring(0, k) + "2" + objFile.substring(k + 1);
			};
			if ((objFile[(int)k] == '/') || (objFile[(int)k] == '\\')) {
				break;
			};
		};
		return objFile + ".2";
	};

	String libraryName(const String &name) {
		if (name.beginWith("lib")) {
			return name.substring(3);
		};
		return name;
	};

	bool isContentChanged(const String &fileName, const String &content) {
		String fileContent;
		if (!Shell::fileExists(fileName)) {
			return true;
		};
		if (!Shell::fileGetContents(fileName, fileContent)) {
			return true;
		};
		return (fileContent != content);
	};

	// Everything that changes the object files of a project, other than the source files
	static String compileFlags(ICompiler *compiler, bool isC, int options, TDynamicArray<String> &define, TDynamicArray<String> &incPath) {
		size_t k;
		String retV = "xyo-cc.flags.1\n";
		retV << ((compiler->type == CompilerType::MSVC) ? "msvc" : "gcc") << "\n";
		retV << (isC ? "c" : "cpp") << "\n";
		retV << optionsTag(options);
		if (options & CompilerOptions::DynamicLibraryXStatic) {
			retV << "X";
		};
		retV << "\n";
		retV << (compiler->isOSWindows ? "W" : "-");
		retV << (compiler->isOSLinux ? "L" : "-");
		retV << (compiler->isOSEmscripten ? "E" : "-");
		retV << (compiler->is32Bit ? "3" : "-");
		retV << (compiler->is64Bit ? "6" : "-");
		retV << (compiler->isStatic ? "S" : "-");
		retV << "\n";
		retV << Shell::getEnv(isC ? "CC" : "CXX") << "\n";
		for (k = 0; k < define.length(); ++k) {
			retV << "D" << define[k] << "\n";
		};
		for (k = 0; k < incPath.length(); ++k) {
			retV << "I" << incPath[k] << "\n";
		};
		return retV;
	};

	static String flagsFilename(const String &projectName, const String &tmpPath, int options, const char *type) {
		String retV = tmpPath;
		retV << Shell::pathSeparator << Shell::getFileName(projectName) << ".";
		retV << optionsTag(options) << "." << type << ".flags";
		return retV;
	};

	static bool saveFlags(const String &fileName, const String &flags) {
		if (!Shell::mkdirFilePath(fileName)) {
			return false;
		};
		return Shell::filePutContents(fileName, flags);
	};

	namespace CompilerMakeWorker {

		class WorkerReturn : public Object {
			public:
				bool value;

				inline WorkerReturn() {
					value = false;
				};
		};

		TPointer<WorkerReturn> transferReturn(WorkerReturn &value) {
			TPointer<WorkerReturn> retV;
			retV.newMemory();
			retV->value = value.value;
			return retV;
		};

		class WorkerParameter : public Object {
			public:
				ICompiler *compiler;
				bool isC;
				String srcFile;
				String objFile;
				int options;
				TDynamicArray<String> define;
				TDynamicArray<String> incPath;
				int index;
				int indexLn;
				bool echoCmd;

				inline WorkerParameter() {
					compiler = nullptr;
					isC = false;
					options = CompilerOptions::None;
					index = 0;
					indexLn = 0;
					echoCmd = false;
				};
		};

		// Strings are reference counted, the worker thread receives its own copy
		TPointer<WorkerParameter> transferParameter(WorkerParameter &value) {
			TPointer<WorkerParameter> retV;
			size_t k;
			retV.newMemory();
			retV->compiler = value.compiler;
			retV->isC = value.isC;
			retV->srcFile = value.srcFile.value();
			retV->objFile = value.objFile.value();
			retV->options = value.options;
			for (k = 0; k < value.define.length(); ++k) {
				retV->define[k] = value.define[k].value();
			};
			for (k = 0; k < value.incPath.length(); ++k) {
				retV->incPath[k] = value.incPath[k].value();
			};
			retV->index = value.index;
			retV->indexLn = value.indexLn;
			retV->echoCmd = value.echoCmd;
			return retV;
		};

		TPointer<WorkerReturn> procedure(WorkerParameter *parameter, TAtomic<bool> &requestToTerminate) {
			TPointer<WorkerReturn> retV;
			retV.newMemory();
			retV->value = false;
			if (parameter) {
				if (parameter->compiler) {
					if (parameter->isC) {
						retV->value = parameter->compiler->cToObj(
						    parameter->srcFile,
						    parameter->objFile,
						    parameter->options,
						    parameter->define,
						    parameter->incPath,
						    parameter->index,
						    parameter->indexLn,
						    parameter->echoCmd);
					} else {
						retV->value = parameter->compiler->cppToObj(
						    parameter->options,
						    parameter->srcFile,
						    parameter->objFile,
						    parameter->define,
						    parameter->incPath,
						    parameter->index,
						    parameter->indexLn,
						    parameter->echoCmd);
					};
				};
			};
			return retV;
		};

	};

	bool compileSources(
	    ICompiler *compiler,
	    bool isC,
	    const String &projectName,
	    const String &tmpPath,
	    int options,
	    TDynamicArray<String> &define,
	    TDynamicArray<String> &incPath,
	    TDynamicArray<String> &incFiles,
	    TDynamicArray<String> &srcFiles,
	    int numThreads,
	    bool echoCmd,
	    bool force,
	    TDynamicArray<String> &objFiles) {
		size_t k, m;
		String objFile;
		TDynamicArray<FileTime> incFilesTime;
		TPointer<CompilerMakeWorker::WorkerParameter> parameter;
		TPointer<CompilerMakeWorker::WorkerReturn> retV;
		WorkerQueue workerQueue;

		workerQueue.setNumberOfThreads(numThreads);

		String flagsFile = flagsFilename(projectName, tmpPath, options, isC ? "c" : "cpp");
		String flags = compileFlags(compiler, isC, options, define, incPath);
		bool flagsChanged = isContentChanged(flagsFile, flags);

		for (k = 0; k < incFiles.length(); ++k) {
			incFilesTime[k].getLastWriteTime(incFiles[k]);
		};

		for (k = 0; k < srcFiles.length(); ++k) {
			objFile = compiler->objFilename(projectName, srcFiles[k], tmpPath, options, (int)(k + 1), (int)srcFiles.length());
			objFiles.push(objFile);

			if (!(force || flagsChanged)) {
				FileTime objTime;
				FileTime srcTime;
				objTime.getLastWriteTime(objFile);
				srcTime.getLastWriteTime(srcFiles[k]);
				// object file is missing or older than a header
				if (!objTime.isChanged(incFilesTime)) {
					if (objTime.compare(srcTime) >= 0) {
						continue;
					};
				};
			};

			parameter.newMemory();
			parameter->compiler = compiler;
			parameter->isC = isC;
			parameter->srcFile = srcFiles[k];
			parameter->objFile = objFile;
			parameter->options = options;
			for (m = 0; m < define.length(); ++m) {
				parameter->define[m] = define[m];
			};
			for (m = 0; m < incPath.length(); ++m) {
				parameter->incPath[m] = incPath[m];
			};
			parameter->index = (int)(k + 1);
			parameter->indexLn = (int)srcFiles.length();
			parameter->echoCmd = echoCmd;
			TWorkerQueue<CompilerMakeWorker::WorkerReturn,
			             CompilerMakeWorker::WorkerParameter,
			             CompilerMakeWorker::transferReturn,
			             CompilerMakeWorker::transferParameter,
			             CompilerMakeWorker::procedure>::add(workerQueue, parameter);
		};

		if (!workerQueue.isEmpty()) {
			if (!workerQueue.process()) {
				return false;
			};
			for (k = 0; k < workerQueue.length(); ++k) {
				retV = TStaticCast<CompilerMakeWorker::WorkerReturn *>(workerQueue.getReturnValue(k));
				if (!retV) {
					return false;
				};
				if (!retV->value) {
					return false;
				};
			};
		};

		if (flagsChanged) {
			return saveFlags(flagsFile, flags);
		};

		return true;
	};

	bool compileResources(
	    ICompiler *compiler,
	    const String &projectName,
	    const String &tmpPath,
	    int options,
	    TDynamicArray<String> &incFiles,
	    TDynamicArray<String> &rcDefine,
	    TDynamicArray<String> &incPathRC,
	    TDynamicArray<String> &rcFiles,
	    bool echoCmd,
	    bool force,
	    TDynamicArray<String> &objFiles) {
		size_t k;
		String objFile;
		TDynamicArray<FileTime> incFilesTime;

		if (rcFiles.isEmpty()) {
			return true;
		};

		String flagsFile = flagsFilename(projectName, tmpPath, options, "rc");
		String flags = compileFlags(compiler, false, options, rcDefine, incPathRC);
		bool flagsChanged = isContentChanged(flagsFile, flags);

		for (k = 0; k < incFiles.length(); ++k) {
			incFilesTime[k].getLastWriteTime(incFiles[k]);
		};

		for (k = 0; k < rcFiles.length(); ++k) {
			objFile = compiler->objFilename(projectName, rcFiles[k], tmpPath, options, (int)(k + 1), (int)rcFiles.length());

			bool forceRc = force || flagsChanged;
			if (!forceRc) {
				FileTime objTime;
				objTime.getLastWriteTime(objFile);
				// object file is missing or older than a header
				forceRc = objTime.isChanged(incFilesTime);
			};

			if (!compiler->makeRcToObj(
			        rcFiles[k],
			        objFile,
			        rcDefine,
			        incPathRC,
			        echoCmd,
			        forceRc)) {
				return false;
			};

			objFiles.push(objFile);
		};

		if (flagsChanged) {
			return saveFlags(flagsFile, flags);
		};

		return true;
	};

};
