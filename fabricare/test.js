// Created by Grigore Stefan <g_stefan@yahoo.com>
// Public domain (Unlicense) <http://unlicense.org>
// SPDX-FileCopyrightText: 2022 Grigore Stefan <g_stefan@yahoo.com>
// SPDX-License-Identifier: Unlicense

messageAction("test");

Shell.mkdirRecursivelyIfNotExists("output/test");
Shell.mkdirRecursivelyIfNotExists("temp");

// ---

exitIf(Shell.execute("output/bin/xyo-cc @input/xyo-cc-x.compile.arguments --output-bin-path=output/test"));
exitIf(Shell.execute("output/bin/xyo-cc @input/xyo-cc-y.compile.json --output-bin-path=output/test"));
