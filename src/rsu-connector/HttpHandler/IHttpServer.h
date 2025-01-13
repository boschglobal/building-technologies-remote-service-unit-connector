// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#ifndef CONNECTOR_HTTPHANDLER_IHTTPSERVER_H_
#define CONNECTOR_HTTPHANDLER_IHTTPSERVER_H_

#include <string>
#include <functional>

#include "common/NonCopyable.h"

/// @brief DTO that is used to forward the HTTP request parameters to the handler and for the handler to return its response to the request.
/// @param RequestPath Path of the request. The host part of the URL and the leading '/' of the path has already been stripped away,
///                    so a request to http://127.0.0.1/some/path would set RequestPath to "some/path".
/// @param RequestBody Body of the HTTP request (for POST requests).
/// @param ResponseCode HTTP response code to return with the response (usually 200).
/// @param ResponseBody HTTP response body to return
struct ServerHandlerDTO
{
    std::string RequestPath;
    std::string RequestBody;
    int32_t ResponseCode     = 0;
    std::string ResponseBody = {};
};

using HttpServerHandlerType = std::function<void( ServerHandlerDTO& )>;

/// @brief Interface for the oatpp HTTP server.
///
/// Instances listen for requests and call the approprate request handler. If no handler is installed, the request is ignored.
struct IHttpServer : private NonCopyable<IHttpServer>
{
    virtual ~IHttpServer() = default;

    /// @brief Run the main loop of this server
    /// This is typically called from a dedicated thread. This call will block until the Stop() method is called.
    virtual void Run() = 0;

    /// @brief Stop the main loop of this server
    /// This causes the Run() method to return, so the thread can be freed.
    virtual void Stop() = 0;

    /// @brief Add a handler for receiving HTTP requests.
    /// @param method HTTP method the handler applies to ("GET" or "POST")
    /// @param path HTTP path (part of the URL after the host and port) the handler applies to, "*" for all paths
    /// @param handler Handler to call when a request matching method and path is received.
    virtual void AddHandler( const std::string& method, const std::string& path, HttpServerHandlerType handler ) = 0;
};

#endif // CONNECTOR_HTTPHANDLER_IHTTPSERVER_H_
