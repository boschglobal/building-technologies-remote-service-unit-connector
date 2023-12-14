// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#ifndef COMMAND_STOPVPNCONNECTION_H
#define COMMAND_STOPVPNCONNECTION_H

#include "ICommand.h"
#include "IVPNActor.h"
#include <memory>
#include <vector>

namespace Command {

/// @brief ICommand instance that stops a VPN connection.
/// This is done by calling on the StopVPN() method on the given actor interface.
class StopVPNConnection : public ICommand
{
public:
    StopVPNConnection() = delete;

    /// @brief Construct the command instance with the given actor interface.
    /// @param actor Interface that is used to stop the VPN connection via StopVPN().
    StopVPNConnection( std::shared_ptr<IVPNActor> actor );

    /// @brief Execute the command with given argument, returning a response.
    /// @param argument Quoted string containing the VPN configuration name.
    /// @param response string where the command stores the response of the format:
    ///                 {
    ///                     "ConnectionClosed" : true,
    ///                     "Message": "VPN stopped."
    ///                 }
    /// @return Always 0. A failure to close the connection will be signalled using the response JSON "ConnectionClosed": false.
    int Run( const std::string& argument, std::string& response );

private:
    std::shared_ptr<IVPNActor> m_actor;
};

} // namespace Command

#endif // COMMAND_STOPVPNCONNECTION_H
