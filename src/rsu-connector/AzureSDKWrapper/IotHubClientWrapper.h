// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#ifndef CONNECTOR_AZURESDKWRAPPER_IOTHUBCLIENTWRAPPER_H_
#define CONNECTOR_AZURESDKWRAPPER_IOTHUBCLIENTWRAPPER_H_

#include <functional>
#include <map>
#include <memory>
#include <vector>

#include "IIotHubClient.h"
#include "common/NonCopyable.h"

using DeviceTwinReportMethod = std::function<void( const std::string& )>;

/// @brief Wrapper for the Azure IoT Hub Service Client
///
/// The IoTHubClientWrapper provides the functionality to send device status and data to the IoT Hub.
/// Furthermore there is the possibility to request and update the Device Twin.
/// Usually the correct way for this client to be constructed is by calling the IotHubClient()
/// method of the IotHubFactory which will handle initializing the Azure C SDK
/// and providing authentication details to it.
class IotHubClientWrapper : public IIotHubClient, private NonCopyable<IotHubClientWrapper>
{
public:
    /// @brief Construct a Iot Hub Client from device authentication.
    /// @param iotHubUri witch receive from @ref ProvisioningClientWrapper.
    /// @param deviceId witch receive from @ref ProvisioningClientWrapper.
    /// @param statusFile specifies where the connection status should be saved.
    IotHubClientWrapper( const std::string& iotHubUri, const std::string& deviceId, const std::string& statusFile );

    /// @brief Construct a new Iot Hub Client by connectionString.
    /// @param connectionString of IoT Hub
    /// @param statusFile specifies where the connection status should be saved.
    IotHubClientWrapper( const std::string& connectionString, const std::string& statusFile );

    /// @brief Destroy the Iot Hub Client Wrapper object.
    virtual ~IotHubClientWrapper() = default;

    /// @brief Enable/disable the Log Trace Option.
    /// @param enable == true activate the logging otherwise  disable.
    void SetLogTraceOption( const bool );

    /// @brief Send new message to IoT Hub.
    /// @param message as data list.
    /// @param properties map of key value properties.
    void SendMessage( const MessageDataList&, const MessagePropertyList&, std::shared_ptr<IMessageLifeTimeTracker> );

    /// @brief Send new message to IoT Hub.
    /// @param message to send.
    /// @param properties map of key value properties.
    void SendMessage( const std::string&, const MessagePropertyList&, std::shared_ptr<IMessageLifeTimeTracker> );

    /// @brief Receive the device twin JSON object for this device.
    /// This will trigger a call to the handler set with SetDeviceTwinHandler(), so register that handler first before
    /// calling this method.
    /// @return true: success, false: error
    bool TriggerGetTwin();

    /// @brief Send new state of the device to IoT Hub.
    /// The state report needs to be a valid JSON object following the structure of the "reported" part of
    /// https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-devguide-device-twins
    /// @param reportedStateJson new device state.
    void SendReportedState( const std::string& reportedStateJson );

    /// @brief Set callback that is called when a remote method invocation is triggered by the Iot Hub.
    /// @param handler callback to install. Its parameters are the method name, the method argument and
    ///                its response string that should be set by the handler. The return value is the status value of the method.
    void SetMethodHandler( std::function<int( const std::string&, const std::string&, std::string& )> handler );

    /// @brief Set callback that is called when a device twin update is sent by the Iot Hub.
    /// @param handler callback to install. Its parameters are the device twin JSON object and whether this is a partial update.
    void SetDeviceTwinHandler( std::function<void( const std::string&, bool )> handler );

private:
    struct IotHubClientWrapperImpl;
    std::shared_ptr<IotHubClientWrapperImpl> _impl;
};

#endif /* CONNECTOR_AZURESDKWRAPPER_IOTHUBCLIENTWRAPPER_H_ */
