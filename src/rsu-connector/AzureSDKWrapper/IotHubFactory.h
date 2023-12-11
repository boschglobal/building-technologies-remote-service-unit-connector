// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#ifndef DEVICECONNECTOR_AZURESDKWRAPPER_IOTHUBFACTORY_H_
#define DEVICECONNECTOR_AZURESDKWRAPPER_IOTHUBFACTORY_H_
#include <memory>
#include "IProvisioningClient.h"
#include "IIotHubClient.h"

class IotHubFactory
{
public:
    IotHubFactory() = delete;
    /// @brief Construct the Azure client factory
    /// This factory wraps the initialization and deinit of the Azure Iot Hub SDK and also sets up the way of authentication that is used.
    /// Only shared access token and certificate authentication is supported with the latter being used when the access token is empty.
    /// @param registrationId Registration id of the device if sharedAccessSignature is used.
    /// @param sharedAccessSignature Shared access signature of the device. Leave this empty to use certificate authentication.
    /// @param certificateFileName Filename of the x509 device certificate in PEM format (must be full chain, e.g. include all CA certificates necessary for validation).
    /// @param keyFileName Filename of the device key in PEM format. Must not have a passphrase set.
    IotHubFactory( const std::string& registrationId,
                   const std::string& sharedAccessSignature,
                   const std::string& certificateFileName,
                   const std::string& keyFileName );
    virtual ~IotHubFactory();

    /// @brief Construct a Device Provisioning Client with given Scope Id.
    /// @param scope The Scope Id assigned to the DPS instance to be connected to.
    std::shared_ptr<IProvisioningClient> ProvisioningClient( const std::string& scope ) const;

    /// @brief Construct a IotHub Client from IotHub URI and DeviceId as returned from the ProvisioningClient.
    /// @param iotHubUri The URI of the IotHub instance.
    /// @param deviceId The device id to be used on this IotHub..
    std::shared_ptr<IIotHubClient> IotHubClient( const std::string& iotHubUri, const std::string& deviceId ) const;

    /// @brief Construct a IotHub Client from connection string as given by the Azure portal. This is only provided for testing purposes.
    /// @param connectionString The IotHub connection string.
    std::shared_ptr<IIotHubClient> IotHubClient( const std::string& connectionString ) const;

private:
    struct IotHubFactoryImpl;
    std::shared_ptr<IotHubFactoryImpl> _impl;
};
#endif // DEVICECONNECTOR_AZURESDKWRAPPER_IOTHUBFACTORY_H_
