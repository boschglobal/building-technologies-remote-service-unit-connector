// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#include "HttpHandlerFactory.h"
#include "HttpServer.h"
#include "HttpClient.h"
#include <oatpp/core/base/Environment.hpp>
#include <oatpp/core/base/memory/MemoryPool.hpp>
#include <spdlog/spdlog.h>

HttpHandlerFactory::HttpHandlerFactory()
{
    oatpp::base::Environment::init();
}

HttpHandlerFactory::~HttpHandlerFactory()
{
    spdlog::debug( "oatpp object count {}", oatpp::base::Environment::getObjectsCount() );
    oatpp::base::Environment::destroy();
}

std::shared_ptr<IHttpServer> HttpHandlerFactory::Server( const std::string& ipAddress, uint16_t port ) const
{
    return std::make_shared<HttpServer>( ipAddress, port );
}

std::shared_ptr<IHttpClient> HttpHandlerFactory::Client( const std::string& ipAddress, uint16_t port ) const
{
    return std::make_shared<HttpClient>( ipAddress, port );
}
