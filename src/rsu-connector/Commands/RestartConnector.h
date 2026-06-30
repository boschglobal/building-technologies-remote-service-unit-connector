// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#ifndef COMMAND_RESTARTCONNECTOR_H_
#define COMMAND_RESTARTCONNECTOR_H_

#include "ICommand.h"
#include <JobScheduler/IJobScheduler.h>
#include <memory>
#include <vector>

namespace Command {

/// @brief ICommand instance that makes the connector exit.
/// This is done by adding a restart task to the given scheduler so the response can be sent back to the Iothub before the exit.
class RestartConnector : public ICommand
{
public:
    RestartConnector() = delete;

    /// @brief Construct the command instance with the given scheduler interface.
    /// @param scheduler Interface that is used to add the task to quit the connector.
    RestartConnector( std::shared_ptr<IJobScheduler> scheduler );

    /// @brief Execute the command with given argument, returning a response.
    /// @param argument Not used.
    /// @param response string where the command stores the response of the format:
    ///                 {
    ///                     "RestartInitiated" : true,
    ///                     "Message": "Restart in progress."
    ///                 }
    /// @return Always 0.
    int Run( const std::string& argument, std::string& response );

private:
    std::shared_ptr<IJobScheduler> m_scheduler;
};

} // namespace Command

#endif // COMMAND_RESTARTCONNECTOR_H_
