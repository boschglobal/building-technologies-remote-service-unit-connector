// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#ifndef COMMAND_GETVPNCONFIGURATIONS_H_
#define COMMAND_GETVPNCONFIGURATIONS_H_

#include "ICommand.h"
#include "IVPNActor.h"
#include <memory>
#include <vector>

namespace Command {

/// @brief ICommand instance that queries the known VPN connection configurations.
/// This is done by calling on the GetVPNConfigurations() method on the given actor interface.
class GetVPNConfigurations : public ICommand
{
public:
    GetVPNConfigurations() = delete;

    /// @brief Construct the command instance with the given actor interface.
    /// @param actor Interface that is used to query the VPN configurations via GetVPNConfigurations().
    GetVPNConfigurations( std::shared_ptr<IVPNActor> actor );

    /// @brief Execute the command with given argument, returning a response.
    /// @param argument Not used.
    /// @param response string where the command stores the response of the format:
    ///                 [ {
    ///                     "Name": "gateway",
    ///                     "IsConnected": false
    ///                 }, ... ]
    /// @return Always 0. A failure to query the VPN connection configurations will be signalled with an empty response.
    int Run( const std::string& argument, std::string& response );

private:
    std::shared_ptr<IVPNActor> m_actor;
};

} // namespace Command

#endif // COMMAND_GETVPNCONFIGURATIONS_H_
