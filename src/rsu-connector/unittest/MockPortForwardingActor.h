// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#include "Commands/IPortForwardingActor.h"

struct MockPortForwardingActor : public Command::IPortForwardingActor
{
    bool AddRedirectCalled{ false };
    bool AddRedirectReturn{ true };
    bool ClearRedirectCalled{ false };
    bool ClearRedirectReturn{ true };
    bool DeleteRedirectCalled{ false };
    bool DeleteRedirectReturn{ true };
    bool GetPortForwardingsCalled{ false };
    bool GetPortForwardingsReturn{ true };
    bool ReloadFirewallCalled{ false };
    bool ReloadFirewallReturn{ true };
    PortForwardingConfiguration RedirectConfig;
    std::string DeleteRedirectName;
    std::vector<PortForwardingConfiguration> GetForwardingsResponse;

    bool AddRedirect( const PortForwardingConfiguration& config )
    {
        AddRedirectCalled = true;
        RedirectConfig    = config;
        return AddRedirectReturn;
    }

    bool ClearRedirect()
    {
        ClearRedirectCalled = true;
        return ClearRedirectReturn;
    }

    bool DeleteRedirect( const std::string& name )
    {
        DeleteRedirectCalled = true;
        DeleteRedirectName   = name;
        return DeleteRedirectReturn;
    }

    bool GetPortForwardings( std::vector<PortForwardingConfiguration>& config )
    {
        GetPortForwardingsCalled = true;
        config                   = GetForwardingsResponse;
        return GetPortForwardingsReturn;
    }

    bool ReloadFirewall()
    {
        ReloadFirewallCalled = true;
        return ReloadFirewallReturn;
    }
};
