// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#ifndef COMMAND_CLEARPORTFORWARDINGS_H_
#define COMMAND_CLEARPORTFORWARDINGS_H_

#include "ICommand.h"
#include "IPortForwardingActor.h"
#include <memory>
#include <vector>

namespace Command {

/// @brief ICommand instance that clears all port forwardings.
/// This is done by calling on the ClearRedirect() method on the given actor interface.
class ClearPortForwardings : public ICommand
{
public:
    ClearPortForwardings() = delete;

    /// @brief Construct the command instance with the given actor interface.
    /// @param actor Interface that is used to clear all redirects via ClearRedirect() and then reload the firewall with ReloadFirewall().
    ClearPortForwardings( std::shared_ptr<IPortForwardingActor> actor );

    /// @brief Execute the command with given argument, returning a response.
    /// @param argument Not used.
    /// @param response string where the command stores the response of the format:
    ///                 {
    ///                     "PortForwardingCleared" : true,
    ///                     "Message": "Port forwardings cleared."
    ///                 }
    /// @return Always 0. A failure to clear the port forwardings will be signalled using the response JSON "PortForwardingCleared": false.
    int Run( const std::string& argument, std::string& response );

private:
    std::shared_ptr<IPortForwardingActor> m_actor;
};

} // namespace Command

#endif // COMMAND_CLEARPORTFORWARDINGS_H_
