// C++ Compiler Command Driver
// Copyright (c) 2022 Grigore Stefan <g_stefan@yahoo.com>
// MIT License (MIT) <http://opensource.org/licenses/MIT>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: MIT

#include <XYO/CPPCompilerCommandDriver/CompilerOptions.hpp>

namespace XYO::CPPCompilerCommandDriver {

	int filterOptions(int options) {
		int retV = 0;
		if (options & CompilerOptions::Debug) {
			retV |= CompilerOptions::Debug;
		} else {
			retV |= CompilerOptions::Release;
		};
		if (options & CompilerOptions::CRTStatic) {
			retV |= CompilerOptions::CRTStatic;
		} else {
			retV |= CompilerOptions::CRTDynamic;
		};
		if (options & CompilerOptions::StaticLibrary) {
			retV |= CompilerOptions::StaticLibrary;
		} else {
			retV |= CompilerOptions::DynamicLibrary;
		};
		if (options & CompilerOptions::DynamicLibraryXStatic) {
			retV &= ~CompilerOptions::CRTDynamic;
			retV &= ~CompilerOptions::StaticLibrary;
			retV |= CompilerOptions::DynamicLibraryXStatic;
			retV |= CompilerOptions::DynamicLibrary;
			retV |= CompilerOptions::CRTStatic;
		};
		return retV;
	};

};
