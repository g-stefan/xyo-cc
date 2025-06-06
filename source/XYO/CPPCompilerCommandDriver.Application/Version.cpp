// C++ Compiler Command Driver
// Copyright (c) 2020-2025 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2020-2025 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/CPPCompilerCommandDriver.Application/Version.hpp>
#include <XYO/CPPCompilerCommandDriver.Application/Version.rh>

namespace XYO::CPPCompilerCommandDriver::Application::Version {

	static const char *version_ = XYO_CPPCOMPILERCOMMANDDRIVER_APPLICATION_VERSION_STR;
	static const char *build_ = XYO_CPPCOMPILERCOMMANDDRIVER_APPLICATION_VERSION_STR_BUILD;
	static const char *versionWithBuild_ = XYO_CPPCOMPILERCOMMANDDRIVER_APPLICATION_VERSION_STR_WITH_BUILD;
	static const char *datetime_ = XYO_CPPCOMPILERCOMMANDDRIVER_APPLICATION_VERSION_STR_DATETIME;

	const char *version() {
		return version_;
	};
	const char *build() {
		return build_;
	};
	const char *versionWithBuild() {
		return versionWithBuild_;
	};
	const char *datetime() {
		return datetime_;
	};

};
