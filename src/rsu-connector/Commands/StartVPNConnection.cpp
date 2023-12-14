// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#include "StartVPNConnection.h"
#include <regex>
#include <oatpp/core/macro/codegen.hpp>
#include <oatpp/parser/json/mapping/ObjectMapper.hpp>
#include <spdlog/spdlog.h>

namespace Command {

class _VPNConnectionStatusDTO : public oatpp::DTO
{
#include OATPP_CODEGEN_BEGIN( DTO ) ///< Begin DTO codegen section
    DTO_INIT( _VPNConnectionStatusDTO, DTO )
    DTO_FIELD( Boolean, ConnectionEstablished );
    DTO_FIELD( String, RemoteConnectionIPAddress );
    DTO_FIELD( String, Message );
#include OATPP_CODEGEN_END( DTO ) ///< End DTO codegen section
};

StartVPNConnection::StartVPNConnection( std::shared_ptr<IVPNActor> actor ) : m_actor( actor )
{
    if ( !actor )
    {
        throw std::runtime_error( "StartVPNConnection without actor" );
    }
}

int StartVPNConnection::Run( const std::string& argument, std::string& response )
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
    auto responseDTO  = _VPNConnectionStatusDTO::createShared();

    std::string ipAddress{ "" };

    if ( !std::regex_match( name, std::regex( "[a-zA-Z0-9_]+" ) ) )
    {
        responseDTO->ConnectionEstablished = false;
        responseDTO->Message               = "Name argument can only contain letters, digits and underscores.";
    }
    else
    {
        auto result = m_actor->StartVPN( name, ipAddress );

        switch ( result )
        {
            case Command::IVPNActor::ConnectionStatusVPN::Established:
                responseDTO->ConnectionEstablished = true;
                responseDTO->Message               = "VPN started.";
                break;

            case Command::IVPNActor::ConnectionStatusVPN::AlreadyRunning:
                responseDTO->ConnectionEstablished = true;
                responseDTO->Message               = "VPN was already started.";
                break;

            case Command::IVPNActor::ConnectionStatusVPN::NoConfiguration:
                responseDTO->ConnectionEstablished = false;
                responseDTO->Message               = "No configuration of this name was found.";
                break;

            default:
                responseDTO->ConnectionEstablished = false;
                responseDTO->Message               = "VPN could not be started.";
        }
    }
    // Still has to be set, otherwise the null will cause an error in the iothub JSON parser.
    responseDTO->RemoteConnectionIPAddress = ipAddress.c_str();
    response                               = objectMapper->writeToString( responseDTO )->std_str();
    return 0;
}

} // namespace Command
