// C++ Compiler Command Driver
// Copyright (c) 2020-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2020-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_CPPCOMPILERCOMMANDDRIVER_DEPENDENCY_HPP
#define XYO_CPPCOMPILERCOMMANDDRIVER_DEPENDENCY_HPP

#ifndef XYO_SYSTEM_HPP
#	include <XYO/System.hpp>
#endif

// -- Export

#ifndef XYO_CPPCOMPILERCOMMANDDRIVER_INTERNAL
#	ifdef XYO_CC_INTERNAL
#		define XYO_CPPCOMPILERCOMMANDDRIVER_INTERNAL
#	endif
#endif

#ifdef XYO_CPPCOMPILERCOMMANDDRIVER_INTERNAL
#	define XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT XYO_LIBRARY_EXPORT
#else
#	define XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT XYO_LIBRARY_IMPORT
#endif

namespace XYO::CPPCompilerCommandDriver {
	using namespace XYO::System;
};

#endif
