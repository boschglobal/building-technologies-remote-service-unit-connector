// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#ifndef CONNECTOR_HTTPHANDLER_HTTPHANDLERFACTORY_H_
#define CONNECTOR_HTTPHANDLER_HTTPHANDLERFACTORY_H_
#include <memory>
#include "IHttpClient.h"
#include "IHttpServer.h"

/// @brief Factory for HttpClient and HttpServer.
/// This factory exists so common initialization can be performed before a client or server is constructed
/// and resources can be freed after.
class HttpHandlerFactory
{
public:
    /// @brief Create the factory.
    HttpHandlerFactory();
    virtual ~HttpHandlerFactory();

    /// @brief Construct a HTTP server, listening to request on the given IP and port.
    /// @param ipAddress IP Address the server listens to. Can be a dot separated IPv4 or a colon separated IPv6 address.
    ///                  Note that an IPv6 address like "::0" will also receive IPv4 requests via IPv4 in IPv6 emulation.
    /// @param port Port the server is listening to
    std::shared_ptr<IHttpServer> Server( const std::string& ipAddress, uint16_t port ) const;

    /// @brief Construct a HTTP client, sending request to the given IP and port.
    /// @param ipAddress IP Address the client connects to. Can be a dot separated IPv4 or a colon separated IPv6 address.
    /// @param port Port the client connects to.
    std::shared_ptr<IHttpClient> Client( const std::string& ipAddress, uint16_t port ) const;
};
#endif // CONNECTOR_HTTPHANDLER_HTTPHANDLERFACTORY_H_
