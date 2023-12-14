// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#ifndef DEVICECONNECTOR_AZURESDKWRAPPER_IPROVISIONINGCLIENT_H_
#define DEVICECONNECTOR_AZURESDKWRAPPER_IPROVISIONINGCLIENT_H_
#include "IProvisioningClient.h"
#include "common/NonCopyable.h"

/// @brief Wrapper for the Azure Device Provisining Service (DPS).
///
/// The DPS is used as a single connection point for all devices. It assigns an IotHub to the device according to configured parameters.
/// The Scope ID references the DPS to be used. Authentification is done using the builtin TPM module or its simulator.
/// If the call to Register() succeeds, the DeviceId and IotHubUri can be read and used to build an IotHubClientWrapper from them.
struct IProvisioningClient : private NonCopyable<IProvisioningClient>
{
    virtual ~IProvisioningClient() = default;

    /// @brief Perform the registration to the DPS.
    /// @return true: registration successful, false: registration failed.
    /// @exception runtime_error thrown when the connection to the TPM failed or the DPS client could not be constructed.
    virtual bool Register() = 0;

    /// @brief After a successful Register, this getter returns the DeviceId required to connect to the assigned IotHub.
    virtual const std::string& DeviceId() const = 0;

    /// @brief After a successful Register, this getter returns the IotHubUri required to connect to the assigned IotHub.
    virtual const std::string& IotHubUri() const = 0;
};

#endif // CONNECTOR_AZURESDKWRAPPER_IPROVISIONINGCLIENT_H_
