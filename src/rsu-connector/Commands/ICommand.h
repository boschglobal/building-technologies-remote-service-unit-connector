// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#ifndef COMMAND_ICOMMAND_H_
#define COMMAND_ICOMMAND_H_

#include <string>

namespace Command {

/// @brief Interface class to a generic command that takes a string argument and returns a string response an an error code.
struct ICommand
{
    virtual ~ICommand() = default;

    /// @brief Run the command
    /// @param argument String containing the commands argument, expected to be in JSON format.
    /// @param response String containing the commands response, expected to be in JSON format.
    /// @return error code, by convention 0 is used for no error.
    virtual int Run( const std::string& argument, std::string& response ) = 0;
};

} // namespace Command

#endif // COMMAND_ICOMMAND_H_
