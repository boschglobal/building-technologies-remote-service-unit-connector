// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#ifndef COMMAND_IPORTFORWARDINGACTOR_H_
#define COMMAND_IPORTFORWARDINGACTOR_H_
#include <string>
#include <vector>

namespace Command {

/// @brief Interface class using which the various commands will query and affect the devices port forwardings.
/// Note that all changes to port forwardings will require the firewall to be reloaded using ReloadFirewall() to take effect.
struct IPortForwardingActor
{
    /// @brief DTO that contains the information for one port forwarding.
    struct PortForwardingConfiguration
    {
        /// @brief Name of the port forwarding.
        std::string Name;
        /// @brief Source port of the forwarding.
        std::string SourcePort;
        /// @brief Destination IP address of the forwarding.
        std::string DestinationIP;
        /// @brief Destination port of the forwarding.
        std::string DestinationPort;
    };

    /// @brief Add a port forwarding to the device configuration.
    /// @param config Configuration for the port forwarding to be added.
    /// @return true for success, false for error.
    virtual bool AddRedirect( const PortForwardingConfiguration& config ) = 0;

    /// @brief Clear all port forwardings from the device configuration.
    /// @return true for success, false for error.
    virtual bool ClearRedirect() = 0;

    /// @brief Remove one port forwarding from the device configuration.
    /// @param name Name of the port forwarding to remove.
    /// @return true for success, false for error.
    virtual bool DeleteRedirect( const std::string& name ) = 0;

    /// @brief Query active port forwardings from the device.
    /// @param config Reference to vector where the port forwarding configurations will be entered.
    /// @return true for success, false for error.
    virtual bool GetPortForwardings( std::vector<PortForwardingConfiguration>& config ) = 0;

    /// @brief Reloads the firewall configuration. This will make changes to the port forwardings configuration active on the device.
    /// @return true for success, false for error.
    virtual bool ReloadFirewall() = 0;
};

} // namespace Command

#endif // COMMAND_IPORTFORWARDINGACTOR_H_
