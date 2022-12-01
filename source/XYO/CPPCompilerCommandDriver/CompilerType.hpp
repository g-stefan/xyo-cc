// C++ Compiler Command Driver
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_CPPCOMPILERCOMMANDDRIVER_COMPILERTYPE_HPP
#define XYO_CPPCOMPILERCOMMANDDRIVER_COMPILERTYPE_HPP

#ifndef XYO_CPPCOMPILERCOMMANDDRIVER_DEPENDENCY_HPP
#	include <XYO/CPPCompilerCommandDriver/Dependency.hpp>
#endif

namespace XYO::CPPCompilerCommandDriver {

	struct CompilerType {
			static const int None = 0;
			static const int MSVC = 1;
			static const int GCC = 2;
	};

};

#endif
