// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#include "ClearPortForwardings.h"

#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/parser/json/mapping/ObjectMapper.hpp>
#include <spdlog/spdlog.h>

namespace Command {

class _PortForwardingClearedStatusDTO : public oatpp::DTO
{
#include OATPP_CODEGEN_BEGIN( DTO ) ///< Begin DTO codegen section
    DTO_INIT( _PortForwardingClearedStatusDTO, DTO )
    DTO_FIELD( Boolean, PortForwardingCleared );
    DTO_FIELD( String, Message );
#include OATPP_CODEGEN_END( DTO ) ///< End DTO codegen section
};

ClearPortForwardings::ClearPortForwardings( std::shared_ptr<IPortForwardingActor> actor ) : m_actor( actor )
{
    if ( !actor )
    {
        throw std::runtime_error( "ClearPortForwardings without actor" );
    }
}

int ClearPortForwardings::Run( const std::string& argument, std::string& response )
{
    auto objectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
    auto responseDTO  = _PortForwardingClearedStatusDTO::createShared();
    if ( m_actor->ClearRedirect() )
    {
        m_actor->ReloadFirewall();
        responseDTO->PortForwardingCleared = true;
        responseDTO->Message               = "Port forwardings cleared.";
    }
    else
    {
        responseDTO->PortForwardingCleared = false;
        responseDTO->Message               = "Clearing of port forwarding failed.";
    }
    response = objectMapper->writeToString( responseDTO )->std_str();
    return 0;
}

} // namespace Command
