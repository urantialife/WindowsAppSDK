/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once

#include <wil/tracelogging.h>
#include <TraceLoggingProvider.h>
#include <telemetry\MicrosoftTelemetry.h>

#define TRACELOGGING_LAUNCHADAPTER_PROVIDER_NAME "Microsoft.Windows.ApplicationModel.pip"

TRACELOGGING_DECLARE_PROVIDER(g_pipTraceLoggingProvider);

// GUID for PIP trace logging provider: {65c056f3-458d-4df1-9a03-3303733f671f}
TRACELOGGING_DEFINE_PROVIDER(
    g_pipTraceLoggingProvider,
    TRACELOGGING_LAUNCHADAPTER_PROVIDER_NAME,
    (0x65c056f3, 0x458d, 0x4df1, 0x9a, 0x03, 0x33, 0x03, 0x73, 0x3f, 0x67, 0x1f),
    TraceLoggingOptionMicrosoftTelemetry());

class PipProvider : public wil::TraceLoggingProvider
{
    //65c056f3-458d-4df1-9a03-3303733f671f
    IMPLEMENT_TRACELOGGING_CLASS(PipProvider, TRACELOGGING_LAUNCHADAPTER_PROVIDER_NAME,
        (0x65c056f3, 0x458d, 0x4df1, 0x9a, 0x03, 0x33, 0x03, 0x73, 0x3f, 0x67, 0x1f));
};
