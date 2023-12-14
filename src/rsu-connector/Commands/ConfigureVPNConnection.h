// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#ifndef COMMAND_CONFIGUREVPNCONNECTION_H
#define COMMAND_CONFIGUREVPNCONNECTION_H

#include "ICommand.h"
#include "IVPNActor.h"
#include <memory>
#include <vector>

namespace Command {

/// @brief ICommand instance that configures a VPN connection.
/// This is done by calling on the ConfigureVPN() method on the given actor interface.
class ConfigureVPNConnection : public ICommand
{
public:
    ConfigureVPNConnection() = delete;

    /// @brief Construct the command instance with the given actor interface.
    /// @param actor Interface that is used to configure the VPN via ConfigureVPN().
    ConfigureVPNConnection( std::shared_ptr<IVPNActor> actor );

    /// @brief Execute the command with given argument, returning a response.
    /// @param argument Json encoded string that supplies the parameters for the call:
    ///                 {
    ///                     "Name": "gateway",
    ///                     "VPNconfig": "......"
    ///                 }
    ///                 The Name configures the VPN connection name. The VPNconfig is a base64 encoded file containing the configuration.
    /// @param response string where the command stores the response of the format:
    ///                 {
    ///                     "VPNConfigurationChanged" : true,
    ///                     "Message": "VPN configuration created/updated."
    ///                 }
    /// @return Always 0. A failure to configure the VPN connection will be signalled using the response JSON "VPNConfigurationChanged": false.
    int Run( const std::string& argument, std::string& response );

private:
    std::shared_ptr<IVPNActor> m_actor;
};

} // namespace Command

#endif // COMMAND_CONFIGUREVPNCONNECTION_H
