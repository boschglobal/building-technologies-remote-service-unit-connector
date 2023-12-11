// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#ifndef COMMAND_IVPNACTOR_H_
#define COMMAND_IVPNACTOR_H_
#include <string>
#include <vector>

namespace Command {

/// @brief Interface class using which the various commands will query and affect the devices VPN connections.
/// Note that configuration changes will only take effect when the connection is stopped and started again afterwards.
struct IVPNActor
{
    /// @brief DTO used to query VPN connections.
    struct VPNConfiguration
    {
        /// @brief Name of the VPN connection.
        std::string Name;
        /// @brief true: connection is currently active, false: connection is inactive.
        bool IsConnected;
    };

    /// @brief Status returned by StartVPN() and StopVPN().
    enum ConnectionStatusVPN
    {
        /// @brief The connection has been successfully established, success for StartVPN.
        Established,
        /// @brief The connection has been successfully disconnected, success for StopVPN.
        Disconnected,
        /// @brief No configuration of this name was found.
        NoConfiguration,
        /// @brief An error occurred during start or stop.
        Error,
        /// @brief The connection was already running.
        AlreadyRunning,
        /// @brief The connection was already stopped.
        AlreadyStopped
    };

    /// @brief Start a VPN connection
    /// @param name Name of the connnection configuration.
    /// @param ipAddress On success, the local IP Address of the connection shall be returned here.
    /// @return Connection Status as described above.
    virtual ConnectionStatusVPN StartVPN( const std::string& name, std::string& ipAddress ) = 0;

    /// @brief Stop a VPN connection
    /// @param name Name of the connnection configuration.
    /// @return Connection Status as described above.
    virtual ConnectionStatusVPN StopVPN( const std::string& name ) = 0;

    /// @brief Provide the configuration of a VPN connection
    /// If the name is reused, the configuration shall overwrite the old configuration.
    /// @param name Name of the connnection configuration.
    /// @param config String containing ovpn configuration file contents.
    /// @return true: success, false: error
    virtual bool ConfigureVPN( const std::string& name, const std::string& config ) = 0;

    /// @brief Query the currently known configurations and their status.
    /// @param config Array of VPN configurations and whether they are active or not.
    /// @return true: success, false: error
    virtual bool GetVPNConfigurations( std::vector<VPNConfiguration>& config ) = 0;
};

} // namespace Command

#endif // COMMAND_IVPNACTOR_H_
