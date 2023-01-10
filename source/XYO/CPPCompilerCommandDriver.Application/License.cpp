// C++ Compiler Command Driver
// Copyright (c) 2020-2023 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2020-2023 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/CPPCompilerCommandDriver.Application/License.hpp>

namespace XYO::CPPCompilerCommandDriver::Application::License {

	const char *license() {
		return XYO::CPPCompilerCommandDriver::License::license();
	};

	const char *shortLicense() {
		return XYO::CPPCompilerCommandDriver::License::shortLicense();
	};

};
