// C++ Compiler Command Driver
// Copyright (c) 2020-2024 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2020-2024 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_CPPCOMPILERCOMMANDDRIVER_ICOMPILER_HPP
#define XYO_CPPCOMPILERCOMMANDDRIVER_ICOMPILER_HPP

#ifndef XYO_CPPCOMPILERCOMMANDDRIVER_DEPENDENCY_HPP
#	include <XYO/CPPCompilerCommandDriver/Dependency.hpp>
#endif

#ifndef XYO_CPPCOMPILERCOMMANDDRIVER_COMPILEROPTIONS_HPP
#	include <XYO/CPPCompilerCommandDriver/CompilerOptions.hpp>
#endif

#ifndef XYO_CPPCOMPILERCOMMANDDRIVER_COMPILERTYPE_HPP
#	include <XYO/CPPCompilerCommandDriver/CompilerType.hpp>
#endif

namespace XYO::CPPCompilerCommandDriver {

	class ICompiler : public virtual Object {
			XYO_PLATFORM_INTERFACE(ICompiler);

		public:
			int type;
			bool isOSWindows;
			bool isOSLinux;
			bool is32Bit;
			bool is64Bit;

			virtual String objFilename(
			    const String &project,
			    const String &fileName,
			    const String &tmpPath,
			    int options,
			    int index,
			    int indexLn) = 0;

			virtual bool cppToObj(
			    int options,
			    String cppFile,
			    String objFile,
			    TDynamicArray<String> &cppDefine,
			    TDynamicArray<String> &incPath,
			    int index,
			    int indexLn,
			    bool echoCmd) = 0;

			virtual bool makeObjToLib(
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
			    bool force = false) = 0;

			virtual bool makeObjToExe(
			    String exeName,
			    String binPath,
			    String tmpPath,
			    int options,
			    TDynamicArray<String> &objFiles,
			    TDynamicArray<String> &libDependencyPath,
			    TDynamicArray<String> &libDependency,
			    bool echoCmd,
			    bool force = false) = 0;

			virtual bool rcToRes(
			    String rcFile,
			    String resFile,
			    TDynamicArray<String> &rcDefine,
			    TDynamicArray<String> &incPath,
			    bool echoCmd) = 0;

			virtual bool resToObj(
			    String resFile,
			    String objFile,
			    bool echoCmd) = 0;

			virtual bool makeRcToObj(
			    String rcFile,
			    String objFile,
			    TDynamicArray<String> &rcDefine,
			    TDynamicArray<String> &incPath,
			    bool echoCmd,
			    bool force = false) = 0;

			virtual bool makeCppToLib(
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
			    bool force = false) = 0;

			virtual bool makeCppToExe(
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
			    bool force = false) = 0;

			virtual bool cToObj(
			    String cppFile,
			    String objFile,
			    int options,
			    TDynamicArray<String> &cDefine,
			    TDynamicArray<String> &incPath,
			    int index,
			    int indexLn,
			    bool echoCmd) = 0;

			virtual bool makeCToLib(
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
			    bool force = false) = 0;

			virtual bool makeCToExe(
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
			    bool force = false) = 0;
	};
};

#endif
