<<<<<<< Updated upstream
/********************************************************
=======
﻿/********************************************************
>>>>>>> Stashed changes
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#pragma once

<<<<<<< Updated upstream
#include <wil/tracelogging.h>
#include <TraceLoggingProvider.h>
#include <telemetry\MicrosoftTelemetry.h>

#define TRACELOGGING_LAUNCHADAPTER_PROVIDER_NAME "Microsoft.Windows.ApplicationModel.pip"

TRACELOGGING_DECLARE_PROVIDER(g_pipTraceLoggingProvider);

=======
#include <TraceLoggingProvider.h>

// TODO: enable telemetry
#define ENABLE_TELEMETRY 0

#if ENABLE_TELEMETRY
#ifndef DOWNLEVEL_PRIOR_TO_WIN8
#include <telemetry/MicrosoftTelemetry.h>
#include <telemetry/MicrosoftTelemetryPrivacy.h>
#endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif
#define TRACELOGGING_LAUNCHADAPTER_PROVIDER_NAME "Microsoft.Windows.ApplicationModel.pip"
#ifdef __cplusplus
}
#endif

TRACELOGGING_DECLARE_PROVIDER(g_pipTraceLoggingProvider);

#if ENABLE_TELEMETRY
>>>>>>> Stashed changes
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
<<<<<<< Updated upstream
=======
#endif
>>>>>>> Stashed changes
