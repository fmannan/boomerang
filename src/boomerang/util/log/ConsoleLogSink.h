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


#include "boomerang/ifc/ILogSink.h"


/**
 * Log sink for logging to stdout.
 */
class ConsoleLogSink : public ILogSink
{
public:
    virtual ~ConsoleLogSink() override = default;

    /// \copydoc ILogSink::write
    virtual void write(const QString &s) override;

    /// \copydoc ILogSink::flush
    virtual void flush() override;
};
