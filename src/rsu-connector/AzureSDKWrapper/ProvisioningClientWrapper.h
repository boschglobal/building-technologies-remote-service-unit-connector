// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#ifndef CONNECTOR_PROVISIONINGCLIENTWRAPPER_H_
#define CONNECTOR_PROVISIONINGCLIENTWRAPPER_H_

#include "common/NonCopyable.h"

#include "IProvisioningClient.h"
#include "ProxySettings.h"

/// @brief Wrapper for the Azure Device Provisining Service (DPS).
///
/// The DPS is used as a single connection point for all devices. It assigns an IotHub to the device according to configured parameters.
/// The Scope ID references the DPS to be used. Authentication is done using the builtin TPM module or its simulator.
/// If the call to Register() succeeds, the DeviceId and IotHubUri can be read and used to build an IotHubClientWrapper from them.
class ProvisioningClientWrapper : public IProvisioningClient, private NonCopyable<ProvisioningClientWrapper>
{
public:
    /// @brief Construct a Device Provisioning Service Client with given Scope Id.
    /// @param scope The Scope Id assigned to the DPS instance to be connected to.
    /// @param proxy Optional HTTP proxy. When Enabled(), DPS connects through it.
    ProvisioningClientWrapper( const std::string& scope, const ProxySettings& proxy = {} );

    virtual ~ProvisioningClientWrapper() = default;

    /// @brief Perform the registration to the DPS.
    /// @return true: registration successful, false: registration failed.
    /// @exception runtime_error thrown when the connection to the TPM failed or the DPS client could not be constructed.
    bool Register();

    /// @brief After a successful Register, this getter returns the DeviceId required to connect to the assigned IotHub.
    const std::string& DeviceId() const;

    /// @brief After a successful Register, this getter returns the IotHubUri required to connect to the assigned IotHub.
    const std::string& IotHubUri() const;

private:
    struct ProvisioningClientWrapperImpl;
    std::shared_ptr<ProvisioningClientWrapperImpl> _impl;
};

#endif /* CONNECTOR_PROVISIONINGCLIENTWRAPPER_H_ */
