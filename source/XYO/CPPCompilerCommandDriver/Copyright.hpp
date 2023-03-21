// C++ Compiler Command Driver
// Copyright (c) 2020-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2020-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_CPPCOMPILERCOMMANDDRIVER_COPYRIGHT_HPP
#define XYO_CPPCOMPILERCOMMANDDRIVER_COPYRIGHT_HPP

#ifndef XYO_CPPCOMPILERCOMMANDDRIVER_DEPENDENCY_HPP
#	include <XYO/CPPCompilerCommandDriver/Dependency.hpp>
#endif

namespace XYO::CPPCompilerCommandDriver::Copyright {

	XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT std::string copyright();
	XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT std::string publisher();
	XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT std::string company();
	XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT std::string contact();

};

#endif
