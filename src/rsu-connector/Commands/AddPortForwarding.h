// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#ifndef COMMAND_ADDPORTFORWARDING_H_
#define COMMAND_ADDPORTFORWARDING_H_

#include "ICommand.h"
#include "IPortForwardingActor.h"
#include <memory>
#include <vector>

namespace Command {

/// @brief ICommand instance that adds a port forwarding to the firewall configuration.
/// This is done by calling on the AddForwarding() method on the given actor interface.
class AddPortForwarding : public ICommand
{
public:
    AddPortForwarding() = delete;

    /// @brief Construct the command instance with the given actor interface.
    /// @param actor Interface that is used to add the redirect via AddRedirect() and then reload the firewall with ReloadFirewall().
    AddPortForwarding( std::shared_ptr<IPortForwardingActor> actor );

    /// @brief Execute the command with given argument, returning a response.
    /// @param argument Json encoded string that supplies the parameters for the call:
    ///                 {
    ///                     "Name": "ForwardOne",
    ///                     "SourceIP": "",
    ///                     "SourcePort": "25201",
    ///                     "DestinationIP": "192.168.6.9",
    ///                     "DestinationPort": "25200"
    ///                 }
    ///                 The SourceIP argument is ignored by this implementation.
    /// @param response string where the command stores the response of the format:
    ///                 {
    ///                     "PortForwardingActive" : true,
    ///                     "Message": "Port forwarding added."
    ///                 }
    /// @return Always 0. A failure to add the port forwarding will be signalled using the response JSON "PortForwardingActive": false.
    int Run( const std::string& argument, std::string& response );

private:
    std::shared_ptr<IPortForwardingActor> m_actor;
};

} // namespace Command

#endif // COMMAND_ADDPORTFORWARDING_H_
