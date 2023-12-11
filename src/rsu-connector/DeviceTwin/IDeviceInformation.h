// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#ifndef CONNECTOR_IDEVICEINFORMATION_H_
#define CONNECTOR_IDEVICEINFORMATION_H_
#include <string>

/// @brief Interface the DeviceTwin uses to query the device information for updating the device state in the IotHub.
struct IDeviceInformation
{
    virtual ~IDeviceInformation() = default;

    /// @brief Return the name set on the device
    virtual std::string Name() = 0;

    /// @brief Set the device name
    /// @param name new device name.
    /// @return true: success, false: error
    virtual bool SetName( const std::string& name ) = 0;

    /// @brief Query the serial number of the device.
    /// @return serial number as an ascii string.
    virtual std::string Serial() = 0;

    /// @brief Query the host name of the device.
    /// @return Host name of the device..
    virtual std::string Hostname() = 0;

    /// @brief Query the type of the device.
    /// @return Type of the device..
    virtual std::string Type() = 0;

    /// @brief Query the firmware version of the device.
    /// @return Firmware version of the device..
    virtual std::string FirmwareVersion() = 0;

    /// @brief Query the MAC address of the main network interface.
    /// @return MAC address of the device..
    virtual std::string MACAddress() = 0;

    /// @brief Query whether a VPN connection is currently active.
    /// @return true: VPN connection is active, false: VPN is not active.
    virtual bool VPNConnectionActive() = 0;
};

#endif // CONNECTOR_IDEVICEINFORMATION_H_
