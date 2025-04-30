// C++ Compiler Command Driver
// Copyright (c) 2020-2025 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2020-2025 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/CPPCompilerCommandDriver.Application/Copyright.hpp>
#include <XYO/CPPCompilerCommandDriver.Application/Copyright.rh>

namespace XYO::CPPCompilerCommandDriver::Application::Copyright {

	static const char *copyright_ = XYO_CPPCOMPILERCOMMANDDRIVER_APPLICATION_COPYRIGHT;
	static const char *publisher_ = XYO_CPPCOMPILERCOMMANDDRIVER_APPLICATION_PUBLISHER;
	static const char *company_ = XYO_CPPCOMPILERCOMMANDDRIVER_APPLICATION_COMPANY;
	static const char *contact_ = XYO_CPPCOMPILERCOMMANDDRIVER_APPLICATION_CONTACT;

	const char *copyright() {
		return copyright_;
	};

	const char *publisher() {
		return publisher_;
	};

	const char *company() {
		return company_;
	};

	const char *contact() {
		return contact_;
	};

};
