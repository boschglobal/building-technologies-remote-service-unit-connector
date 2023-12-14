// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#include "ConfigureVPNConnection.h"
#include <regex>
#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/parser/json/mapping/ObjectMapper.hpp>
#include <spdlog/spdlog.h>
#include <base64.h>

namespace Command {

class _ConfigureVPNConnectionDTO : public oatpp::DTO
{
#include OATPP_CODEGEN_BEGIN( DTO ) ///< Begin DTO codegen section
    DTO_INIT( _ConfigureVPNConnectionDTO, DTO )
    DTO_FIELD( String, Name );
    DTO_FIELD( String, VPNconfig );
#include OATPP_CODEGEN_END( DTO ) ///< End DTO codegen section
};

class _ConfigureVPNConnectionResponseDTO : public oatpp::DTO
{
#include OATPP_CODEGEN_BEGIN( DTO ) ///< Begin DTO codegen section
    DTO_INIT( _ConfigureVPNConnectionResponseDTO, DTO )
    DTO_FIELD( Boolean, VPNConfigurationChanged );
    DTO_FIELD( String, Message );
#include OATPP_CODEGEN_END( DTO ) ///< End DTO codegen section
};

ConfigureVPNConnection::ConfigureVPNConnection( std::shared_ptr<IVPNActor> actor ) : m_actor( actor )
{
    if ( !actor )
    {
        throw std::runtime_error( "ConfigureVPNConnection without actor" );
    }
}

int ConfigureVPNConnection::Run( const std::string& argument, std::string& response )
{
    auto objectMapper = oatpp::parser::json::mapping::ObjectMapper::createShared();
    auto params       = objectMapper->readFromString<oatpp::Object<_ConfigureVPNConnectionDTO>>( argument.c_str() );
    auto responseDTO  = _ConfigureVPNConnectionResponseDTO::createShared();
    if ( !params || !params->Name || !params->VPNconfig )
    {
        spdlog::error( "ConfigureVPNConnection missing manadatory parameter." );
        responseDTO->VPNConfigurationChanged = false;
        responseDTO->Message = "VPM configuration could not be created/updated due to missing paramater.";
    }
    else if ( !std::regex_match( params->Name->std_str(), std::regex( "[a-zA-Z0-9_]+" ) ) )
    {
        responseDTO->VPNConfigurationChanged = false;
        responseDTO->Message                 = "Name argument can only contain letters, digits and underscores.";
    }
    else
    {
        std::string binary;
        try
        {
            binary = base64_decode( params->VPNconfig->std_str() );
        }
        catch ( std::exception& e )
        {
            spdlog::error( "ConfigureVPNConnection: invalid base64 config: {}", e.what() );
        }
        if ( binary.size() == 0 )
        {
            responseDTO->VPNConfigurationChanged = false;
            responseDTO->Message                 = "VPM configuration invalid.";
        }
        else if ( m_actor->ConfigureVPN( params->Name->std_str(), binary ) )
        {
            responseDTO->VPNConfigurationChanged = true;
            responseDTO->Message                 = "VPN configuration created/updated.";
        }
        else
        {
            responseDTO->VPNConfigurationChanged = false;
            responseDTO->Message                 = "VPM configuration could not be created/updated.";
        }
    }
    response = objectMapper->writeToString( responseDTO )->std_str();
    return 0;
}

} // namespace Command
