﻿#include "pch.h"
#include <testdef.h>
#include <iostream>
#include <wil/win32_helpers.h>
#include "WindowsAppRuntime.Test.AppModel.h"

namespace winrt
{
    using namespace winrt::Microsoft::Windows::AppLifecycle;
    using namespace winrt::Windows::ApplicationModel::Activation;
    using namespace winrt::Windows::Foundation;
    using namespace winrt::Microsoft::Windows::ToastNotifications;
}

bool VerifyFailedRegisterActivatorUsingNullClsid()
{
    try
    {
        auto activationInfo = winrt::ToastActivationInfo::CreateFromActivationGuid(winrt::guid(GUID_NULL));

        winrt::ToastNotificationManager::Default().RegisterActivator(activationInfo);
    }
    catch (...)
    {
        return winrt::to_hresult() == E_INVALIDARG;
    }

    return false;
}

bool VerifyFailedRegisterActivatorUsingNullClsid_Unpackaged()
{
    try
    {
        auto activationInfo = winrt::ToastActivationInfo::CreateFromActivationGuid(winrt::guid(GUID_NULL));

        winrt::ToastNotificationManager::Default().RegisterActivator(activationInfo);
    }
    catch (...)
    {
        return winrt::to_hresult() == E_ILLEGAL_METHOD_CALL;
    }

    return false;
}
bool VerifyFailedRegisterActivatorUsingNullAssets()
{
    try
    {
        auto activationInfo = winrt::ToastActivationInfo::CreateFromToastAssets(nullptr);

        winrt::ToastNotificationManager::Default().RegisterActivator(activationInfo);
    }
    catch (...)
    {
        return winrt::to_hresult() == E_ILLEGAL_METHOD_CALL;
    }

    return false;
}

bool VerifyFailedRegisterActivatorUsingNullAssets_Unpackaged()
{
    try
    {
        auto activationInfo = winrt::ToastActivationInfo::CreateFromToastAssets(nullptr);

        winrt::ToastNotificationManager::Default().RegisterActivator(activationInfo);
    }
    catch (...)
    {
        return winrt::to_hresult() == E_INVALIDARG;
    }

    return false;
}

bool VerifyRegisterActivatorandUnRegisterActivatorUsingClsid()
{
    auto activationInfo = winrt::ToastActivationInfo::CreateFromActivationGuid(winrt::guid("1940DBA9-0F64-4F0D-8A4B-5D207B812E61"));

    winrt::ToastNotificationManager::Default().RegisterActivator(activationInfo);

    winrt::ToastNotificationManager::Default().UnregisterActivator();

    return true;
}

bool VerifydRegisterActivatoandUnRegisterActivatorUsingAssets_Unpackaged()
{
    winrt::ToastAssets assets(L"ToastNotificationApp", winrt::Uri{ LR"(C:\Windows\System32\WindowsSecurityIcon.png)" });

    auto activationInfo = winrt::ToastActivationInfo::CreateFromToastAssets(assets);

    winrt::ToastNotificationManager::Default().RegisterActivator(activationInfo);

    winrt::ToastNotificationManager::Default().UnregisterActivator();

    return true;
}

bool VerifyFailedMultipleRegisterActivatorUsingSameClsid()
{
    try
    {
        auto activationInfo = winrt::ToastActivationInfo::CreateFromActivationGuid(winrt::guid("1940DBA9-0F64-4F0D-8A4B-5D207B812E61"));

        winrt::ToastNotificationManager::Default().RegisterActivator(activationInfo);

        winrt::ToastNotificationManager::Default().RegisterActivator(activationInfo);
    }
    catch (...)
    {
        return winrt::to_hresult() == E_INVALIDARG;
    }
    
    return false;
}

std::string unitTestNameFromLaunchArguments(const winrt::ILaunchActivatedEventArgs& launchArgs)
{
    std::string unitTestName = to_string(launchArgs.Arguments());
    auto argStart = unitTestName.rfind(" ");
    if (argStart != std::wstring::npos)
    {
        unitTestName = unitTestName.substr(argStart + 1);
    }

    return unitTestName;
}

std::map<std::string, bool(*)()> const& GetSwitchMapping()
{
    static std::map<std::string, bool(*)()> switchMapping = {
        { "VerifyFailedRegisterActivatorUsingNullClsid", &VerifyFailedRegisterActivatorUsingNullClsid },
        { "VerifyFailedRegisterActivatorUsingNullClsid_Unpackaged", &VerifyFailedRegisterActivatorUsingNullClsid_Unpackaged},
        { "VerifyFailedRegisterActivatorUsingNullAssets", &VerifyFailedRegisterActivatorUsingNullAssets },
        { "VerifyRegisterActivatorandUnRegisterActivatorUsingClsid", &VerifyRegisterActivatorandUnRegisterActivatorUsingClsid },
        { "VerifydRegisterActivatoandUnRegisterActivatorUsingAssets_Unpackaged", &VerifydRegisterActivatoandUnRegisterActivatorUsingAssets_Unpackaged },
        { "VerifyFailedMultipleRegisterActivatorUsingSameClsid", &VerifyFailedMultipleRegisterActivatorUsingSameClsid }
    };
    return switchMapping;
}

bool runUnitTest(std::string unitTest)
{
    auto const& switchMapping = GetSwitchMapping();
    auto it = switchMapping.find(unitTest);
    if (it == switchMapping.end())
    {
        return false;
    }

    return it->second();
}


int main() try
{
    bool testResult = false;
    auto scope_exit = wil::scope_exit([&] {
        ::Test::Bootstrap::CleanupBootstrap();
        });

    ::Test::Bootstrap::SetupBootstrap();

    auto args = winrt::AppInstance::GetCurrent().GetActivatedEventArgs();
    auto kind = args.Kind();

    if (kind == winrt::ExtendedActivationKind::Launch)
    {
        auto unitTest = unitTestNameFromLaunchArguments(args.Data().as<winrt::ILaunchActivatedEventArgs>());
        std::cout << unitTest << std::endl;

        testResult = runUnitTest(unitTest);
    }

    return testResult ? 0 : 1; // We want 0 to be success and 1 failure
}
catch (...)
{
    std::cout << winrt::to_string(winrt::to_message()) << std::endl;

    return 1; // in the event of unhandled test crash
}
