//  Microsoft Windows
//  Copyright (c) Microsoft Corporation. All rights reserved.

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <unknwn.h>
#include <Windows.h>

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <wil\cppwinrt.h>
#include <wil\result.h>
#include <wil\resource.h>
#include <wil\winrt.h>
#include <wil\result_macros.h>
#include <wil\win32_helpers.h>

#include <wrl\client.h>
#include <wrl\wrappers\corewrappers.h>

#include <appmodel.h>
#include <winnt.h>
#include <winrt\Windows.ApplicationModel.h>
#include <winrt\Windows.Foundation.h>
#include <winrt\Windows.Foundation.Collections.h>
#include <winrt\Windows.Storage.h>
#include <winrt\Windows.Management.Core.h>
#include <winrt\Windows.Management.Deployment.h>
