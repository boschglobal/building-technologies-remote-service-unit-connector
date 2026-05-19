// Copyright (c) 2026 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#ifndef CONNECTOR_AZURESDKWRAPPER_PROXYSETTINGS_H_
#define CONNECTOR_AZURESDKWRAPPER_PROXYSETTINGS_H_

#include <cstdint>
#include <string>

/// @brief Proxy authentication scheme.
///
/// `Basic` keeps the historical behaviour: `Proxy_Username`/`Proxy_Password` are sent as
/// `Authorization: Basic` (or no header at all when both are empty).
/// `Negotiate` activates SPNEGO/Kerberos via libcurl's GSSAPI support; the username/password
/// remain available as a Basic fallback if the proxy doesn't offer Negotiate.
enum class ProxyAuthMethod
{
    Basic,
    Negotiate,
};

/// @brief HTTP CONNECT proxy configuration for the Azure IoT Hub and DPS clients.
///
/// An empty Host or zero Port means "no proxy"; Username/Password are optional even when Host/Port are set.
/// When `AuthMethod == Negotiate` and `Keytab` is non-empty, the connector exports
/// `KRB5_CLIENT_KTNAME` so libcurl/GSSAPI acquires the proxy ticket from that keytab.
struct ProxySettings
{
    std::string Host;
    uint16_t Port{ 0 };
    std::string Username;
    std::string Password;
    ProxyAuthMethod AuthMethod{ ProxyAuthMethod::Basic };
    std::string Keytab;

    bool Enabled() const noexcept { return !Host.empty() && Port != 0; }
};

#endif // CONNECTOR_AZURESDKWRAPPER_PROXYSETTINGS_H_
