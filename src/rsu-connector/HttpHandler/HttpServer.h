// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#ifndef CONNECTOR_HTTPHANDLER_HTTPSERVER_H_
#define CONNECTOR_HTTPHANDLER_HTTPSERVER_H_

#include <memory>
#include "IHttpServer.h"

/// @brief Wrapper for the oatpp http server.
///
/// Listens on the given ip and port for requests, calling the approprate handler. If no handler is installed, the request is ignored.
class HttpServer : public IHttpServer
{
public:
    HttpServer() = delete;

    /// @brief Construct the HTTP service listening under the given ip and port.
    /// @param ip IP the server should listen under. If ip contains a ':', the address is interpreted as IPv6, otherwise as IPv4.
    ///           Note that an IPv6 service like "::0" will also receive IPv4 requests via IPv4 in IPv6 emulation.
    /// @param port Port the server should listen to.
    HttpServer( const std::string& ip, uint16_t port );

    virtual ~HttpServer();

    /// @brief Run the main loop of this server
    /// This is typically called from a dedicated thread. This call will block until the Stop() method is called.
    /// Note that the server loop uses a timeout of 1s to check for stop condition, so this method may return
    /// up to 1s after Stop() was called.
    virtual void Run();

    /// @brief Stop the main loop of this server
    /// This causes the Run() method to return, so the thread can be freed.
    virtual void Stop();

    /// @brief Add a handler for receiving HTTP requests.
    /// @param method HTTP method the handler applies to ("GET" or "POST")
    /// @param path HTTP path (part of the URL after the host and port) the handler applies to, "*" for all paths
    /// @param handler Handler to call when a request matching method and path is received.
    virtual void AddHandler( const std::string& method, const std::string& path, HttpServerHandlerType handler );

#if !defined( UNITTEST_PUBLIC_PIMPL )
private:
#endif
    struct HttpServerImpl;
    std::shared_ptr<HttpServerImpl> _impl;
};

#endif // CONNECTOR_HTTPHANDLER_IHTTPSERVER_H_
