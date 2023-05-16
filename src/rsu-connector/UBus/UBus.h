// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#ifndef CONNECTOR_UBUS_H_
#define CONNECTOR_UBUS_H_
#include <string>
#include <memory>
#include <vector>
#include <DeviceTwin/IDeviceInformation.h>
#include <Commands/GetNetworkDetails.h>
#include <Commands/IPortForwardingActor.h>
#include <Commands/IVPNActor.h>

class Ubus : public IDeviceInformation, public Command::INetworkDetailSource, public Command::IPortForwardingActor, public Command::IVPNActor
{
public:

    Ubus(const std::string& firewallZone, const std::string& openVpnLogPath);
    virtual ~Ubus();
    
    std::string Name();
    bool SetName(const std::string& newName);
    std::string Serial();
    std::string Hostname();
    std::string Type();
    std::string FirmwareVersion();
    std::string MACAddress();
    bool VPNConnectionActive();

    bool DeleteRedirect(const std::string& name);
    bool ClearRedirect();
    bool AddRedirect(const Command::IPortForwardingActor::PortForwardingConfiguration& config);
    bool ReloadFirewall();
    ConnectionStatusVPN StartVPN(const std::string& name, std::string& ipAddress);
    ConnectionStatusVPN StopVPN(const std::string& name);
    bool ConfigureVPN(const std::string& name, const std::string& config);
    bool GetNetworkDetails( std::vector<Command::INetworkDetailSource::NetworkDetail>& details);
    bool GetVPNConfigurations( std::vector<Command::IVPNActor::VPNConfiguration>& config);
    bool GetPortForwardings( std::vector<Command::IPortForwardingActor::PortForwardingConfiguration>& config);

#if !defined(UNITTEST_PUBLIC_PIMPL)
private:
#endif

    struct UbusImpl;
    std::shared_ptr<UbusImpl> _impl;
};

#endif // CONNECTOR_UBUS_H_
