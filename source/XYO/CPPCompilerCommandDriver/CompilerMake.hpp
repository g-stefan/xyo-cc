// C++ Compiler Command Driver
// Copyright (c) 2020-2026 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2020-2026 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_CPPCOMPILERCOMMANDDRIVER_COMPILERMAKE_HPP
#define XYO_CPPCOMPILERCOMMANDDRIVER_COMPILERMAKE_HPP

#ifndef XYO_CPPCOMPILERCOMMANDDRIVER_ICOMPILER_HPP
#	include <XYO/CPPCompilerCommandDriver/ICompiler.hpp>
#endif

// Build steps shared by all compilers

namespace XYO::CPPCompilerCommandDriver::CompilerMake {

	// Letters of the build options used in object file names, "RSS", "DDD", ...
	XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT String optionsTag(int options);

	// Replace suffix at the end of value, value is returned unchanged if it does not end with suffix
	XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT String replaceSuffix(const String &value, const String &suffix, const String &newSuffix);

	// Response file of an object file, "x.cpp.o" -> "x.cpp2o", "x.cc.obj" -> "x.cc2obj"
	XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT String commandFilename(const String &objFile);

	// Library name for -l, "libname" -> "name", only the prefix is removed
	XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT String libraryName(const String &name);

	// True if fileName is not found or its content is not content
	XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT bool isContentChanged(const String &fileName, const String &content);

	// Compile source files to object files, only the changed ones, in parallel.
	// A file is compiled if its object file is missing or older than the file or than
	// any of incFiles, or if the compile flags changed from the last build.
	// objFiles receives the object files of all srcFiles.
	XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT bool compileSources(
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
	    TDynamicArray<String> &objFiles);

	// Compile resource files to object files, only the changed ones.
	// objFiles receives the object files of all rcFiles.
	XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT bool compileResources(
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
	    TDynamicArray<String> &objFiles);

};

#endif
