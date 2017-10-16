#pragma region License
/*
 * This file is part of the Boomerang Decompiler.
 *
 * See the file "LICENSE.TERMS" for information on usage and
 * redistribution of this file, and for a DISCLAIMER OF ALL
 * WARRANTIES.
 */
#pragma endregion License
#pragma once

#include <QtTest/QTest>

class FrontendTest : public QObject
{
	Q_OBJECT

private slots:

	/// Test loading the sparc hello world program
	void test1();
};