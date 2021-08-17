﻿// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "pch.h"
#include "PushNotificationChannel.h"
#include "Microsoft.Windows.PushNotifications.PushNotificationChannel.g.cpp"
#include <winrt\Windows.Networking.PushNotifications.h>
#include <winrt\Windows.Foundation.h>
#include "PushNotificationReceivedEventArgs.h"
#include <PushNotificationsLRP_h.h>
#include <iostream>
namespace winrt::Windows
{
    using namespace winrt::Windows::Networking::PushNotifications;
    using namespace winrt::Windows::Foundation;
}
namespace winrt::Microsoft
{
    using namespace winrt::Microsoft::Windows::PushNotifications;
}

namespace winrt::Microsoft::Windows::PushNotifications::implementation
{
    PushNotificationChannel::PushNotificationChannel(winrt::Windows::PushNotificationChannel const& channel) : m_channel(channel) {}

    winrt::Windows::Uri PushNotificationChannel::Uri()
    {
        return winrt::Windows::Uri{ m_channel.Uri() };
    }
    winrt::Windows::DateTime PushNotificationChannel::ExpirationTime()
    {
        return m_channel.ExpirationTime();
    }
    void PushNotificationChannel::Close()
    {
        try
        {
            m_channel.Close();
        }
        catch (...)
        {
            auto channelCloseException = hresult_error(to_hresult());
            if (channelCloseException.code() != HRESULT_FROM_WIN32(ERROR_NOT_FOUND))
            {
                throw hresult_error(to_hresult());
            }
        }
    }

    void PushNotificationChannel::TriggerForeground()
    {
        wil::com_ptr<IWpnLrpPlatform> reunionEndpoint{
            wil::CoCreateInstance<WpnLrpPlatform,
            IWpnLrpPlatform>(CLSCTX_LOCAL_SERVER) };

        reunionEndpoint->InvokeForegroundHandlers(this);

    }

    void PushNotificationChannel::WpnForegroundInvoke(byte* payload, ULONG length)
    {
        m_foregroundHandlers(*this, winrt::make<winrt::Microsoft::Windows::PushNotifications::implementation::PushNotificationReceivedEventArgs>(payload, length));
    }

    winrt::event_token PushNotificationChannel::PushReceived(winrt::Windows::TypedEventHandler<winrt::Microsoft::Windows::PushNotifications::PushNotificationChannel, winrt::Microsoft::Windows::PushNotifications::PushNotificationReceivedEventArgs> handler)
    {
        // if !IsActivatorSupported(ComActivator)
        wil::com_ptr<IWpnLrpPlatform> reunionEndpoint{
            wil::CoCreateInstance<WpnLrpPlatform,
            IWpnLrpPlatform>(CLSCTX_LOCAL_SERVER) };

        m_foregroundHandlers.add(handler);

        char processName[1024];
        GetModuleFileNameExA(GetCurrentProcess(), NULL, processName, sizeof(processName) / sizeof(processName[0]));

        HRESULT hr = reunionEndpoint->RegisterForegroundActivator(this, processName);

        return m_channel.PushNotificationReceived([weak_self = get_weak(), handler](auto&&, auto&& args)
        {
            auto strong = weak_self.get();
            if (strong)
            {
                handler(*strong, winrt::make<winrt::Microsoft::Windows::PushNotifications::implementation::PushNotificationReceivedEventArgs>(args));
            };
        });

    }

    void PushNotificationChannel::PushReceived(winrt::event_token const& token) noexcept
    {
        // if !IsActivatorSupported(ComActivator) { m_event.remove(token); }

        m_channel.PushNotificationReceived(token);
    }

    /*
    ~PushNotificationChannel::PushNotificationChannel() noexcept
    {
        // if !IsActivatorSupported(ComActivator)
        /*wil::com_ptr<IWpnLrpPlatform> reunionEndpoint{
            wil::CoCreateInstance<WpnLrpPlatform,
            IWpnLrpPlatform>(CLSCTX_LOCAL_SERVER) };*/
            //char processName[1024];
            //GetModuleFileNameExA(GetCurrentProcess(), NULL, processName, sizeof(processName) / sizeof(processName[0]));

            // reunionEndpoint->UnregisterForegroundActivator(this, processName);
    //}
}
