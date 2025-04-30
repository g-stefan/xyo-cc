// C++ Compiler Command Driver
// Copyright (c) 2020-2025 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2020-2025 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#ifndef XYO_CPPCOMPILERCOMMANDDRIVER_APPLICATION_VERSION_HPP
#define XYO_CPPCOMPILERCOMMANDDRIVER_APPLICATION_VERSION_HPP

#ifndef XYO_CPPCOMPILERCOMMANDDRIVER_DEPENDENCY_HPP
#	include <XYO/CPPCompilerCommandDriver/Dependency.hpp>
#endif

namespace XYO::CPPCompilerCommandDriver::Application::Version {

	const char *version();
	const char *build();
	const char *versionWithBuild();
	const char *datetime();

};

#endif
