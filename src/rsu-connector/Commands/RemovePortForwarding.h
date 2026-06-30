// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#ifndef COMMAND_REMOVEPORTFORWARDING_H_
#define COMMAND_REMOVEPORTFORWARDING_H_

#include "ICommand.h"
#include "IPortForwardingActor.h"
#include <memory>
#include <vector>

namespace Command {

/// @brief ICommand instance that removes a port forwarding.
/// This is done by calling on the DeleteRedirect() method on the given actor interface.
class RemovePortForwarding : public ICommand
{
public:
    RemovePortForwarding() = delete;

    /// @brief Construct the command instance with the given actor interface.
    /// @param actor Interface that is used to remove the redirect via DeleteRedirect() and then reload the firewall with ReloadFirewall().
    RemovePortForwarding( std::shared_ptr<IPortForwardingActor> actor );

    /// @brief Execute the command with given argument, returning a response.
    /// @param argument Quoted string containing the name of the port forwarding.
    /// @param response string where the command stores the response of the format:
    ///                 {
    ///                     "PortForwardingRemoved" : true,
    ///                     "Message": "Port forwarding removed."
    ///                 }
    /// @return Always 0. A failure to remove the port forwarding will be signalled using the response JSON "PortForwardingRemoved": false.
    int Run( const std::string& argument, std::string& response );

private:
    std::shared_ptr<IPortForwardingActor> m_actor;
};

} // namespace Command

#endif // COMMAND_REMOVEPORTFORWARDING_H_
