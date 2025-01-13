// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#include "AddPortForwarding.h"
#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/parser/json/mapping/ObjectMapper.hpp>
#include <spdlog/spdlog.h>

namespace Command {

class _AddPortForwardingConfigurationDTO : public oatpp::DTO
{
#include OATPP_CODEGEN_BEGIN( DTO ) ///< Begin DTO codegen section
    DTO_INIT( _AddPortForwardingConfigurationDTO, DTO )
    DTO_FIELD( String, Name );
    DTO_FIELD( String, SourceIP );
    DTO_FIELD( String, SourcePort );
    DTO_FIELD( String, DestinationIP );
    DTO_FIELD( String, DestinationPort );
#include OATPP_CODEGEN_END( DTO ) ///< End DTO codegen section
};

class _AddPortForwardingConfigurationStatusDTO : public oatpp::DTO
{
#include OATPP_CODEGEN_BEGIN( DTO ) ///< Begin DTO codegen section
    DTO_INIT( _AddPortForwardingConfigurationStatusDTO, DTO )
    DTO_FIELD( Boolean, PortForwardingActive );
    DTO_FIELD( String, Message );
#include OATPP_CODEGEN_END( DTO ) ///< End DTO codegen section
};

AddPortForwarding::AddPortForwarding( std::shared_ptr<IPortForwardingActor> actor ) : m_actor( actor )
{
    if ( !actor )
    {
        throw std::runtime_error( "AddPortForwarding without actor" );
    }
}

int AddPortForwarding::Run( const std::string& argument, std::string& response )
{
    auto objectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
    auto responseDTO  = _AddPortForwardingConfigurationStatusDTO::createShared();

    oatpp::Object<_AddPortForwardingConfigurationDTO> params{ nullptr };
    try
    {
        params = objectMapper->readFromString<oatpp::Object<_AddPortForwardingConfigurationDTO>>( argument.c_str() );
    }
    catch ( oatpp::parser::ParsingError& e )
    {
        spdlog::error( "AddPortForwarding parse error at {}, code {}: {}",
                       e.getPosition(),
                       e.getCode(),
                       e.getMessage()->std_str() );
        responseDTO->PortForwardingActive = false;
        responseDTO->Message              = "Port forwarding could not be added due to parse error.";
        response                          = objectMapper->writeToString( responseDTO )->std_str();
        return 0;
    }

    if ( !params || !params->Name || !params->SourcePort || !params->DestinationIP || !params->DestinationPort )
    {
        spdlog::error( "AddPortForwarding missing manadatory parameter." );
        responseDTO->PortForwardingActive = false;
        responseDTO->Message              = "Port forwarding could not be added due to missing parameter.";
    }
    else
    {
        if ( m_actor->AddRedirect( IPortForwardingActor::PortForwardingConfiguration{
                     .Name            = params->Name->std_str(),
                     .SourcePort      = params->SourcePort->std_str(),
                     .DestinationIP   = params->DestinationIP->std_str(),
                     .DestinationPort = params->DestinationPort->std_str() } ) )
        {
            m_actor->ReloadFirewall();
            responseDTO->PortForwardingActive = true;
            responseDTO->Message              = "Port forwarding added.";
        }
        else
        {
            responseDTO->PortForwardingActive = false;
            responseDTO->Message              = "Port forwarding could not be added. Is the Name already in use?";
        }
    }
    response = objectMapper->writeToString( responseDTO )->std_str();
    return 0;
}

} // namespace Command
