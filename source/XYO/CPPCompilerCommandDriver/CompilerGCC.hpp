// C++ Compiler Command Driver
// Copyright (c) 2020-2024 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2020-2024 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_CPPCOMPILERCOMMANDDRIVER_COMPILERGCC_HPP
#define XYO_CPPCOMPILERCOMMANDDRIVER_COMPILERGCC_HPP

#ifndef XYO_CPPCOMPILERCOMMANDDRIVER_ICOMPILER_HPP
#	include <XYO/CPPCompilerCommandDriver/ICompiler.hpp>
#endif

namespace XYO::CPPCompilerCommandDriver {

	class CompilerGCC : public virtual ICompiler {
		public:
			XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT CompilerGCC();

			XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT String objFilename(
			    const String &project,
			    const String &fileName,
			    const String &tmpPath,
			    int options,
			    int index,
			    int indexLn);

			XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT bool cppToObj(
			    int options,
			    String cppFile,
			    String objFile,
			    TDynamicArray<String> &cppDefine,
			    TDynamicArray<String> &incPath,
			    int index,
			    int indexLn,
			    bool echoCmd);

			XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT bool makeObjToLib(
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
			    bool force = false);

			XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT bool makeObjToExe(
			    String exeName,
			    String binPath,
			    String tmpPath,
			    int options,
			    TDynamicArray<String> &objFiles,
			    TDynamicArray<String> &libDependencyPath,
			    TDynamicArray<String> &libDependency,
			    bool echoCmd,
			    bool force = false);

			XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT bool rcToRes(
			    String rcFile,
			    String resFile,
			    TDynamicArray<String> &rcDefine,
			    TDynamicArray<String> &incPath,
			    bool echoCmd);

			XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT bool resToObj(
			    String resFile,
			    String objFile,
			    bool echoCmd);

			XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT bool makeRcToObj(
			    String rcFile,
			    String objFile,
			    TDynamicArray<String> &rcDefine,
			    TDynamicArray<String> &incPath,
			    bool echoCmd,
			    bool force = false);

			XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT bool makeCppToLib(
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
			    bool force = false);

			XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT bool makeCppToExe(
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
			    bool force = false);

			XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT bool cToObj(
			    String cppFile,
			    String objFile,
			    int options,
			    TDynamicArray<String> &cDefine,
			    TDynamicArray<String> &incPath,
			    int index,
			    int indexLn,
			    bool echoCmd);

			XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT bool makeCToLib(
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
			    bool force = false);

			XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT bool makeCToExe(
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
			    bool force = false);
	};
};

#endif
