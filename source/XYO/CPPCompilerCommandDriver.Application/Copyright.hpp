// C++ Compiler Command Driver
// Copyright (c) 2020-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2020-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_CPPCOMPILERCOMMANDDRIVER_APPLICATION_COPYRIGHT_HPP
#define XYO_CPPCOMPILERCOMMANDDRIVER_APPLICATION_COPYRIGHT_HPP

#ifndef XYO_CPPCOMPILERCOMMANDDRIVER_DEPENDENCY_HPP
#	include <XYO/CPPCompilerCommandDriver/Dependency.hpp>
#endif

namespace XYO::CPPCompilerCommandDriver::Application::Copyright {
	const char *copyright();
	const char *publisher();
	const char *company();
	const char *contact();
};

#endif
