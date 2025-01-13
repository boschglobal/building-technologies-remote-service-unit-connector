// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#include "RemovePortForwarding.h"

#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/parser/json/mapping/ObjectMapper.hpp>
#include <spdlog/spdlog.h>

namespace Command {

class _RemovePortForwardingDTO : public oatpp::DTO
{
#include OATPP_CODEGEN_BEGIN( DTO ) ///< Begin DTO codegen section
    DTO_INIT( _RemovePortForwardingDTO, DTO )
    DTO_FIELD( String, Name );
#include OATPP_CODEGEN_END( DTO ) ///< End DTO codegen section
};

class _PortForwardingRemovedStatusDTO : public oatpp::DTO
{
#include OATPP_CODEGEN_BEGIN( DTO ) ///< Begin DTO codegen section
    DTO_INIT( _PortForwardingRemovedStatusDTO, DTO )
    DTO_FIELD( Boolean, PortForwardingRemoved );
    DTO_FIELD( String, Message );
#include OATPP_CODEGEN_END( DTO ) ///< End DTO codegen section
};

RemovePortForwarding::RemovePortForwarding( std::shared_ptr<IPortForwardingActor> actor ) : m_actor( actor )
{
    if ( !actor )
    {
        throw std::runtime_error( "AddPortForwarding without actor" );
    }
}

int RemovePortForwarding::Run( const std::string& argument, std::string& response )
{
    auto name = argument;
    if ( name[0] == '"' )
    {
        name.erase( 0, 1 );
    }
    if ( name.size() > 0 )
    {
        auto it = name.end() - 1;
        if ( *it == '"' )
        {
            name.erase( it );
        }
    }
    auto objectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
    auto responseDTO  = _PortForwardingRemovedStatusDTO::createShared();

    if ( name.size() == 0 )
    {
        spdlog::error( "RemovePortForwarding missing name argument." );
        responseDTO->PortForwardingRemoved = false;
        responseDTO->Message               = "Port forwarding could not be removed, no name was given.";
    }
    else if ( m_actor->DeleteRedirect( name ) )
    {
        m_actor->ReloadFirewall();
        responseDTO->PortForwardingRemoved = true;
        responseDTO->Message               = "Port forwarding removed.";
    }
    else
    {
        responseDTO->PortForwardingRemoved = false;
        responseDTO->Message               = "Port forwarding could not be removed. Is the Name correct?";
    }
    response = objectMapper->writeToString( responseDTO )->std_str();
    return 0;
}

} // namespace Command
