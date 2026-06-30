// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#include "StopVPNConnection.h"
#include <regex>
#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/parser/json/mapping/ObjectMapper.hpp>
#include <spdlog/spdlog.h>

namespace Command {

class _VPNConnectionCloseDTO : public oatpp::DTO
{
#include OATPP_CODEGEN_BEGIN( DTO ) ///< Begin DTO codegen section
    DTO_INIT( _VPNConnectionCloseDTO, DTO )
    DTO_FIELD( Boolean, ConnectionClosed );
    DTO_FIELD( String, Message );
#include OATPP_CODEGEN_END( DTO ) ///< End DTO codegen section
};

StopVPNConnection::StopVPNConnection( std::shared_ptr<IVPNActor> actor ) : m_actor( actor )
{
    if ( !actor )
    {
        throw std::runtime_error( "StopVPNConnection without actor" );
    }
}

int StopVPNConnection::Run( const std::string& argument, std::string& response )
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
    auto responseDTO  = _VPNConnectionCloseDTO::createShared();
    if ( !std::regex_match( name, std::regex( "[a-zA-Z0-9_]+" ) ) )
    {
        responseDTO->ConnectionClosed = false;
        responseDTO->Message          = "Name argument can only contain letters, digits and underscores.";
    }
    else
    {
        auto result = m_actor->StopVPN( name );
        switch ( result )
        {
            case Command::IVPNActor::ConnectionStatusVPN::Disconnected:
                responseDTO->ConnectionClosed = true;
                responseDTO->Message          = "VPN stopped.";
                break;
            case Command::IVPNActor::ConnectionStatusVPN::AlreadyStopped:
                responseDTO->ConnectionClosed = true;
                responseDTO->Message          = "VPN was already stopped.";
                break;
            case Command::IVPNActor::ConnectionStatusVPN::NoConfiguration:
                responseDTO->ConnectionClosed = false;
                responseDTO->Message          = "No configuration of this name was found.";
                break;
            default:
                responseDTO->ConnectionClosed = false;
                responseDTO->Message          = "VPN could not be stopped.";
        }
    }
    response = objectMapper->writeToString( responseDTO )->std_str();
    return 0;
}

} // namespace Command
