// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#ifndef COMMAND_STARTVPNCONNECTION_H
#define COMMAND_STARTVPNCONNECTION_H

#include "ICommand.h"
#include "IVPNActor.h"
#include <memory>
#include <vector>

namespace Command {

/// @brief ICommand instance that starts a VPN connection.
/// This is done by calling on the StartVPN() method on the given actor interface.
class StartVPNConnection : public ICommand
{
public:
    StartVPNConnection() = delete;

    /// @brief Construct the command instance with the given actor interface.
    /// @param actor Interface that is used to start the VPN via StartVPN().
    StartVPNConnection( std::shared_ptr<IVPNActor> actor );

    /// @brief Execute the command with given argument, returning a response.
    /// @param argument Quoted string containing the VPN configuration name.
    /// @param response string where the command stores the response of the format:
    ///                 {
    ///                     "ConnectionEstablished" : true,
    ///                     "Message": "VPN started.",
    ///                     "RemoteConnectionIPAddress": "..."
    ///                 }
    /// @return Always 0. A failure to start the VPN connection will be signalled using the response JSON "ConnectionEstablished": false.
    int Run( const std::string& argument, std::string& response );

private:
    std::shared_ptr<IVPNActor> m_actor;
};

} // namespace Command

#endif // COMMAND_STARTVPNCONNECTION_H
