// C++ Compiler Command Driver
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_CPPCOMPILERCOMMANDDRIVER_COPYRIGHT_HPP
#define XYO_CPPCOMPILERCOMMANDDRIVER_COPYRIGHT_HPP

#ifndef XYO_CPPCOMPILERCOMMANDDRIVER_DEPENDENCY_HPP
#	include <XYO/CPPCompilerCommandDriver/Dependency.hpp>
#endif

namespace XYO::CPPCompilerCommandDriver::Copyright {

	XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT const char *copyright();
	XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT const char *publisher();
	XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT const char *company();
	XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT const char *contact();
	XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT const char *fullCopyright();

};

#endif
