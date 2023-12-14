// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#ifndef COMMAND_GETNETWORKDETAILS_H_
#define COMMAND_GETNETWORKDETAILS_H_

#include "ICommand.h"
#include <memory>
#include <vector>

namespace Command {

struct INetworkDetailSource
{
    virtual ~INetworkDetailSource() = default;

    struct NetworkDetail
    {
        std::string InterfaceName;
        std::string Ipv4Address;
        std::string Ipv6Address;
        std::string Ipv4Gateway;
        std::string Ipv6Gateway;
        std::string MacAddress;
        bool IsVpn{ false };
    };

    virtual bool GetNetworkDetails( std::vector<NetworkDetail>& details ) = 0;
};

/// @brief ICommand instance that fetches the network configuration of the device.
/// This is done by calling on the GetNetworkDetails() method on the given actor interface.
class GetNetworkDetails : public ICommand
{
public:
    GetNetworkDetails() = delete;

    /// @brief Construct the command instance with the given dataSource interface.
    /// @param dataSource Interface that is used to query the network interfaces via GetNetworkDetails().
    GetNetworkDetails( std::shared_ptr<INetworkDetailSource> dataSource );

    /// @brief Execute the command with given argument, returning a response.
    /// @param argument Not used.
    /// @param response string where the command stores the response of the format:
    ///                 [ {
    ///                     "Name": "eth0",
    ///                     "IPv4Address": "192.168.6.7/24",
    ///                     "IPv4Gateway": "192.168.6.1",
    ///                     "IPv6Addresses": "..../64",
    ///                     "IPv6Gateway": "....",
    ///                     "MACAddress": "01:23:45:67:89:ab"
    ///                 }, ... ]
    /// @return Always 0. A failure to query the network configuration will be signalled with an empty response.
    int Run( const std::string& argument, std::string& response );

private:
    std::shared_ptr<INetworkDetailSource> m_dataSource;
};

} // namespace Command

#endif // COMMAND_GETNETWORKDETAILS_H_
