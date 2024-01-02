// C++ Compiler Command Driver
// Copyright (c) 2020-2024 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2020-2024 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_CPPCOMPILERCOMMANDDRIVER_COMPILEROPTIONS_HPP
#define XYO_CPPCOMPILERCOMMANDDRIVER_COMPILEROPTIONS_HPP

#ifndef XYO_CPPCOMPILERCOMMANDDRIVER_DEPENDENCY_HPP
#	include <XYO/CPPCompilerCommandDriver/Dependency.hpp>
#endif

namespace XYO::CPPCompilerCommandDriver {

	struct CompilerOptions {
			static const int None = 0;
			static const int Release = 1;
			static const int Debug = 2;
			static const int CRTDynamic = 4;
			static const int CRTStatic = 8;
			static const int StaticLibrary = 16;
			static const int DynamicLibrary = 32;
			static const int DynamicLibraryXStatic = 64;
	};

	XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT int filterOptions(int options);

};

#endif
