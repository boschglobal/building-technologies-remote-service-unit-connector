// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#ifndef DEVICECONNECTOR_AZURESDKWRAPPER_CUSTOMHSM_H_
#define DEVICECONNECTOR_AZURESDKWRAPPER_CUSTOMHSM_H_
#include <memory>
#include <vector>
#include <string>

/// @brief This class provides a custom hardware security module interface to the Azure Iot C SDK.
/// Using this interface the SDK will request authentication information for accessing the IotHub
/// and the Device Provisioning Service. Currently it can provide either a Shared Access Signature
/// or a x509 device certificate. For access to work using the shared access signature the device
/// Registration Id (as set inside the Individual Registration tab on the DPS) has to be provided.
/// For the x509 certificate authentication, the private key of the device also needs to be provided.
/// Both certificate and key shall be in PEM format. The key must not have a passphrase set.
/// If both Shared Access Signature and certificate authentication has been provided, SAS will be used
///
/// Note: Only RSA keys have been tested.
class CustomHsm
{
public:
    CustomHsm()  = default;
    ~CustomHsm() = default;

    /// @brief Set the Registration Id for shared access signature authentification.
    /// Also requires Shared Access Signature to be set using setSas().
    /// @param dpsRegistrationId Registration Id as set in the DPS.
    void setRegistrationId( const std::string& dpsRegistrationId );

    /// @brief Set the Shared Access Signature for shared access signature authentification.
    /// Also requires Registration Id to be set using setRegistrationId().
    /// @param dpsSharedAccessSignature Shared Access Signature as set in the DPS.
    void setSas( const std::string& dpsSharedAccessSignature );

    /// @brief Set the full chain Device Certificate for certificate authentication.
    /// Also requires the Device Key to be set using setKey().
    /// The full chain certificate is generated by appending all relevant certification authority
    /// certificates to the device certificate so that the whole chain can be validated.
    /// @param dpsCertificate x509 Device Certificate in PEM format.
    bool setCertificate( const std::vector<char>& dpsCertificate );

    /// @brief Set the Device Key for certificate authentication.
    /// Also requires the Device Certificate to be set using setCertificate().
    /// @param dpsKey x509 Device Certificate in PEM format.
    bool setKey( const std::vector<char>& dpsKey );
};
#endif // DEVICECONNECTOR_AZURESDKWRAPPER_CUSTOMHSM_H_