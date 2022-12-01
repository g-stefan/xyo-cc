// C++ Compiler Command Driver
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_CPPCOMPILERCOMMANDDRIVER_VERSION_HPP
#define XYO_CPPCOMPILERCOMMANDDRIVER_VERSION_HPP

#ifndef XYO_CPPCOMPILERCOMMANDDRIVER_DEPENDENCY_HPP
#	include <XYO/CPPCompilerCommandDriver/Dependency.hpp>
#endif

namespace XYO::CPPCompilerCommandDriver::Version {

	XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT const char *version();
	XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT const char *build();
	XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT const char *versionWithBuild();
	XYO_CPPCOMPILERCOMMANDDRIVER_EXPORT const char *datetime();

};

#endif
