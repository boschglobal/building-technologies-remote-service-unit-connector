// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#include "Commands/IVPNActor.h"

struct MockVPNActor : public Command::IVPNActor
{
    bool StartVPNCalled{ false };
    ConnectionStatusVPN StartVPNReturn{ Established };
    bool StopVPNCalled{ false };
    ConnectionStatusVPN StopVPNReturn{ Disconnected };
    bool GetPortForwardingsCalled{ false };
    bool GetPortForwardingsReturn{ true };
    bool GetVPNConfigurationsCalled{ false };
    bool GetVPNConfigurationsReturn{ true };
    std::string StartVPNName;
    std::string StartVPNAddress{ "1.2.3.7" };
    std::string StopVPNName;
    std::vector<VPNConfiguration> GetVPNResponse;

    ConnectionStatusVPN StartVPN( const std::string& name, std::string& ipAddress )
    {
        StartVPNCalled = true;
        StartVPNName   = name;
        ipAddress      = StartVPNAddress;
        return StartVPNReturn;
    }

    ConnectionStatusVPN StopVPN( const std::string& name )
    {
        StopVPNCalled = true;
        StopVPNName   = name;
        return StopVPNReturn;
    }

    bool GetVPNConfigurations( std::vector<VPNConfiguration>& config )
    {
        GetVPNConfigurationsCalled = true;
        config                     = GetVPNResponse;
        return GetVPNConfigurationsReturn;
    }
};