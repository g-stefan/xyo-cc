// C++ Compiler Command Driver
// Copyright (c) 2020-2025 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2020-2025 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/CPPCompilerCommandDriver/Copyright.hpp>
#include <XYO/CPPCompilerCommandDriver/Copyright.rh>

namespace XYO::CPPCompilerCommandDriver::Copyright {

	static const char *copyright_ = XYO_CPPCOMPILERCOMMANDDRIVER_COPYRIGHT;
	static const char *publisher_ = XYO_CPPCOMPILERCOMMANDDRIVER_PUBLISHER;
	static const char *company_ = XYO_CPPCOMPILERCOMMANDDRIVER_COMPANY;
	static const char *contact_ = XYO_CPPCOMPILERCOMMANDDRIVER_CONTACT;

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
