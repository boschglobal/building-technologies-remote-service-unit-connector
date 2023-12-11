// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#ifndef CONNECTOR_METHODROUTER_H_
#define CONNECTOR_METHODROUTER_H_

#include "IIotHubClient.h"
#include "common/NonCopyable.h"

/// @brief Allows to register handlers for IotHub method calls
/// The router will discard method calls until a matching handler has been registered using RegisterMethodHandler().
/// The user is expected to forward method invokation triggered by the IotHUb to ReceiveMethodCall().
class MethodRouter : private NonCopyable<MethodRouter>
{
public:
    /// @brief Construct a method router.
    MethodRouter();

    virtual ~MethodRouter() = default;

    /// @brief Provide a method call.
    /// This will usually be triggered by registering this method via SetMethodHandler() on the IotHub.
    /// @param method Method name that is called. This selects the appropriate handler.
    /// @param arguments The argument (usually a JSON or string object) that are forwarded to the handler.
    /// @param reply The reply (also usually a JSON or string object) that the handler returned.
    virtual int ReceiveMethodCall( const std::string& method, const std::string& arguments, std::string& reply );

    /// @brief Register a method handler for the given method..
    /// @param method Method name the handler shall apply to.
    /// @param handler Handler that shall be called when a method invocation is triggered with ReceiveMethodCall().
    ///                The handler will receive the arguments from the method call and is expected to return a reply.
    virtual void RegisterMethodHandler(
            const std::string& method,
            std::function<int( const std::string& arguments, std::string& reply )> handler );

#if !defined( UNITTEST_PUBLIC_PIMPL )
private:
#endif

    struct MethodRouterImpl;
    std::shared_ptr<MethodRouterImpl> _impl;
};

#endif /* CONNECTOR_METHODROUTER_H_ */
