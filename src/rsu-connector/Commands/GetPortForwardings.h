// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#ifndef COMMAND_GETPORTTFORWARDINGS_H_
#define COMMAND_GETPORTTFORWARDINGS_H_

#include "ICommand.h"
#include "IPortForwardingActor.h"
#include <memory>
#include <vector>

namespace Command {

/// @brief ICommand instance that queries the active port forwardings.
/// This is done by calling on the GetPortForwardings() method on the given actor interface.
class GetPortForwardings : public ICommand
{
public:
    GetPortForwardings() = delete;

    /// @brief Construct the command instance with the given actor interface.
    /// @param actor Interface that is used to query the forwardings via GetPortForwardings().
    GetPortForwardings( std::shared_ptr<IPortForwardingActor> actor );

    /// @brief Execute the command with given argument, returning a response.
    /// @param argument Not used
    /// @param response string where the command stores the response of the format:
    ///                 [ {
    ///                     "Name": "ForwardOne",
    ///                     "SourceIP": "",
    ///                     "SourcePort": "25201",
    ///                     "DestinationIP": "192.168.6.9",
    ///                     "DestinationPort": "25200"
    ///                 }, ... ]
    /// @return Always 0. A failure to query the port forwardings will be signalled with an empty response.
    int Run( const std::string& argument, std::string& response );

private:
    std::shared_ptr<IPortForwardingActor> m_actor;
};

} // namespace Command

#endif // COMMAND_GETPORTTFORWARDINGS_H_
