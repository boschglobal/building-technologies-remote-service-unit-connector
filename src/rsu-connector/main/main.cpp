// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#include <unistd.h>
#include <map>
#include <iomanip>
#include <fstream>
#include <regex>
#include <cstdio>
#include <thread>
#include <cstring>
#include <signal.h>
#include <regex>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/cfg/env.h>
#include <spdlog/fmt/bin_to_hex.h>

#include "AzureSDKWrapper/IotHubFactory.h"
#include "Configuration/Configuration.h"
#include "AzureSDKWrapper/MethodRouter.h"
#include "JobScheduler/JobScheduler.h"
#include "DeviceTwin/DeviceTwin.h"
#include "HttpHandler/HttpHandlerFactory.h"
#include <UBus/UBus.h>

#include <Commands/GetNetworkDetails.h>
#include <Commands/AddPortForwarding.h>
#include <Commands/RemovePortForwarding.h>
#include <Commands/ClearPortForwardings.h>
#include <Commands/GetPortForwardings.h>
#include <Commands/StartVPNConnection.h>
#include <Commands/StopVPNConnection.h>
#include <Commands/ConfigureVPNConnection.h>
#include <Commands/GetVPNConfigurations.h>
#include <Commands/RestartConnector.h>
#include "IotHubOrNull.h"

#include <base64.h>

using namespace std::chrono_literals;

static std::atomic<bool> running{ true };

void intHandler( int dummy )
{
    running = false;
}

int main( int argc, char** argv )
{
    spdlog::cfg::load_env_levels();
    auto err_logger = spdlog::stderr_color_mt( "stderr" );

    signal( SIGINT, intHandler );
    signal( SIGTERM, intHandler );

    std::shared_ptr<Configuration> config;
    std::string configurationFilename;
    if ( argc < 2 )
    {
        configurationFilename = "config/rsu-connector-config.json";
    }
    else
    {
        configurationFilename = argv[1];
    }

    try
    {
        config = std::make_shared<Configuration>( configurationFilename );
    }
    catch ( const std::exception& e )
    {
        spdlog::error( "Could not read configuration {}: {}", configurationFilename, e.what() );
        exit( 1 );
    }

    auto schedTickInterval{ 100ms }; // default value
    try
    {
        schedTickInterval = std::chrono::milliseconds( config->GetIntValue( "jobschedule_tickinternal_ms" ) );
    }
    catch ( const std::exception& e )
    {
        spdlog::error( "Error Config read scheduler tick interval:{} ", e.what() );
    }
    auto sched = std::make_shared<JobScheduler>( schedTickInterval );

    std::map<const std::string, const std::string> properties = { { "messageversion", "0.1" },
                                                                  { "messagetype", "device" } };

    std::shared_ptr<IotHubOrNull> iotHubClient;
    try
    {
        iotHubClient = std::make_shared<IotHubOrNull>( config );
    }
    catch ( const std::exception& e )
    {
        spdlog::error( "Connection to IotHub failed due to exception {}, exiting...", e.what() );
        exit( 1 );
    }
    if ( !iotHubClient )
    {
        spdlog::error( "Connection to IotHub failed, exiting..." );
        exit( 1 );
    }
    std::thread threadIotHub( [iotHubClient]() {
        while ( running )
        {
            try
            {
                auto connected = iotHubClient->Connect();
                if ( connected )
                {
                    break;
                }
                else
                {
                    std::this_thread::sleep_for( 10s );
                }
            }
            catch ( const std::exception& e )
            {
                spdlog::error( "Connection to IotHub failed due to exception {}, exiting...", e.what() );
                exit( 1 );
            }
        }
    } );
    threadIotHub.detach();

    std::string firewallZone = "azurezone";
    try
    {
        firewallZone = config->GetStringValue( "firewall_zone" );
    }
    catch ( const std::exception& e )
    {
        spdlog::info( "Could not read firewall_zone entry: {}", e.what() );
    }

    bool forwardToEffilink{ true };
    try
    {
        forwardToEffilink = config->GetBooleanValue( "forward_to_effilink" );
    }
    catch ( const std::exception& e )
    {
        spdlog::info( "Could not read forward_to_effilink entry: {}", e.what() );
    }

    std::string openVpnLogFile = "/tmp/rsu-connector/openvpn.log";
    try
    {
        openVpnLogFile = config->GetStringValue( "openvpn_log" );
    }
    catch ( const std::exception& e )
    {
        spdlog::warn( "Could not read openvpn_log entry: {}", e.what() );
    }

    auto ubus = std::make_shared<Ubus>( firewallZone, openVpnLogFile );
    auto twin = std::make_shared<DeviceTwin>( ubus );
    iotHubClient->SetDeviceTwinHandler(
            [twin]( const std::string& json, bool partial ) -> void { twin->ReceiveDeviceTwin( json, partial ); } );
    twin->SetReportDeviceTwin( [iotHubClient]( const std::string& json ) -> void {
        spdlog::info( "Updating device twin with '{}'", json );
        iotHubClient->SendReportedState( json );
    } );

    bool scheduleDeviceTwinUpdate{ false };
    sched->Add( "DeviceTwinUpdate", std::chrono::seconds( 1 ), [iotHubClient, &scheduleDeviceTwinUpdate]() {
        if ( scheduleDeviceTwinUpdate )
        {
            spdlog::info( "Triggering device twin update" );
            iotHubClient->TriggerGetTwin();
            scheduleDeviceTwinUpdate = false;
        }
    } );

    auto methodRouter = std::make_shared<MethodRouter>();

    auto cmdGetNetworkDetails = std::make_shared<Command::GetNetworkDetails>( ubus );
    methodRouter->RegisterMethodHandler(
            "GetNetworkDetails",
            [cmdGetNetworkDetails]( const std::string& argument, std::string& response ) -> int {
                return cmdGetNetworkDetails->Run( argument, response );
            } );

    auto cmdAddPortForwarding = std::make_shared<Command::AddPortForwarding>( ubus );
    methodRouter->RegisterMethodHandler(
            "AddPortForwarding",
            [cmdAddPortForwarding]( const std::string& argument, std::string& response ) -> int {
                return cmdAddPortForwarding->Run( argument, response );
            } );

    auto cmdRemovePortForwarding = std::make_shared<Command::RemovePortForwarding>( ubus );
    methodRouter->RegisterMethodHandler(
            "RemovePortForwarding",
            [cmdRemovePortForwarding]( const std::string& argument, std::string& response ) -> int {
                return cmdRemovePortForwarding->Run( argument, response );
            } );

    auto cmdClearPortForwardings = std::make_shared<Command::ClearPortForwardings>( ubus );
    methodRouter->RegisterMethodHandler(
            "ClearPortForwardings",
            [cmdClearPortForwardings]( const std::string& argument, std::string& response ) -> int {
                return cmdClearPortForwardings->Run( argument, response );
            } );

    auto cmdGetPortForwardings = std::make_shared<Command::GetPortForwardings>( ubus );
    methodRouter->RegisterMethodHandler(
            "GetPortForwardings",
            [cmdGetPortForwardings]( const std::string& argument, std::string& response ) -> int {
                return cmdGetPortForwardings->Run( argument, response );
            } );

    auto cmdStartVPNConnection = std::make_shared<Command::StartVPNConnection>( ubus );
    methodRouter->RegisterMethodHandler(
            "StartVPNConnection",
            [cmdStartVPNConnection, &scheduleDeviceTwinUpdate]( const std::string& argument,
                                                                std::string& response ) -> int {
                auto retVal              = cmdStartVPNConnection->Run( argument, response );
                scheduleDeviceTwinUpdate = true;
                return retVal;
            } );

    auto cmdStopVPNConnection = std::make_shared<Command::StopVPNConnection>( ubus );
    methodRouter->RegisterMethodHandler(
            "StopVPNConnection",
            [cmdStopVPNConnection, &scheduleDeviceTwinUpdate]( const std::string& argument,
                                                               std::string& response ) -> int {
                auto retVal              = cmdStopVPNConnection->Run( argument, response );
                scheduleDeviceTwinUpdate = true;
                return retVal;
            } );

    auto cmdConfigureVPNConnection = std::make_shared<Command::ConfigureVPNConnection>( ubus );
    methodRouter->RegisterMethodHandler(
            "ConfigureVPNConnection",
            [cmdConfigureVPNConnection]( const std::string& argument, std::string& response ) -> int {
                return cmdConfigureVPNConnection->Run( argument, response );
            } );

    auto cmdRestartConnector = std::make_shared<Command::RestartConnector>( sched );
    methodRouter->RegisterMethodHandler(
            "RestartConnector",
            [cmdRestartConnector]( const std::string& argument, std::string& response ) -> int {
                return cmdRestartConnector->Run( argument, response );
            } );

    auto cmdGetVPNConfigurations = std::make_shared<Command::GetVPNConfigurations>( ubus );
    methodRouter->RegisterMethodHandler(
            "GetVPNConfigurations",
            [cmdGetVPNConfigurations]( const std::string& argument, std::string& response ) -> int {
                return cmdGetVPNConfigurations->Run( argument, response );
            } );

    iotHubClient->SetMethodHandler(
            [methodRouter]( const std::string& method, const std::string& argument, std::string& response ) -> int {
                spdlog::info( "Method invocation {} param {}", method, argument );
                return methodRouter->ReceiveMethodCall( method, argument, response );
            } );

    auto httpFactory = std::make_shared<HttpHandlerFactory>();

    // Setup http client to Effilink service, this client will be used to forward requests that this service receives for backward compatibility.
    auto client = httpFactory->Client( "127.0.0.1", 9001 );

    // Setup http server that receives the telemetry data and forwards it to the Effilink service and to the IotHub
    auto server6 = httpFactory->Server( "::0", 9000 );

    std::regex validCustomer( "/cm/([0-9]+)" );

    server6->AddHandler(
            "POST",
            "*",
            [client, iotHubClient, forwardToEffilink, validCustomer]( ServerHandlerDTO& dto ) -> void {
                dto.ResponseBody = "";
                dto.ResponseCode = 500;
                std::string responseStatusDescription{ "Internal Error" };
                try
                {
                    spdlog::info( "Received path {} size {}", dto.RequestPath, dto.RequestBody.size() );

                    auto encodedPath = base64_encode(
                            dto.RequestPath,
                            true ); // the path may contain characters that are not handled by the IotHub, so encode it safely.

                    std::map<const std::string, const std::string> properties = { { "messageversion", "0.1" },
                                                                                  { "messagetype", "telemetry" },
                                                                                  { "path", encodedPath } };
                    std::smatch match;
                    if ( std::regex_match( dto.RequestPath, match, validCustomer ) && match.size() > 1 )
                    {
                        std::string customer = match.str( 1 );
                        properties.emplace( std::pair<const std::string, const std::string>( "customer", customer ) );
                    }

                    iotHubClient->SendMessage( dto.RequestBody, properties );

                    if ( forwardToEffilink )
                    {
                        spdlog::info( "Forwarding telemetry to Effilink, path {} size {}",
                                      dto.RequestPath,
                                      dto.RequestBody.size() );

                        // forward response body and status from client request to server response
                        dto.ResponseCode = client->SendRequest( dto.RequestPath,
                                                                dto.RequestBody,
                                                                "application/json",
                                                                dto.ResponseBody,
                                                                responseStatusDescription );
                        spdlog::info( "Forwarded response body {} status <{}-{}>",
                                      dto.ResponseBody,
                                      dto.ResponseCode,
                                      responseStatusDescription );
                    }
                    else
                    {
                        spdlog::debug( "Effilink forwarding disabled." );
                        dto.ResponseCode          = 200;
                        responseStatusDescription = "OK";
                    }
                }
                catch ( std::exception& e )
                {
                    spdlog::error( "Exception occurred during message handling: {}", e.what() );
                }
            } );

    std::thread thread6( [&]() {
        spdlog::info( "server thread start." );
        server6->Run();
        spdlog::info( "server thread stop." );
    } );

    std::thread ticker( [&]() {
        while ( running )
        {
            sched->Tick();
            std::this_thread::sleep_for( schedTickInterval );
        }
    } );

    while ( running )
    {
        auto retVal = pause();
        spdlog::info( "Pause call returned {}.", retVal );
    }

    ticker.join();
    server6->Stop();
    thread6.join();
    iotHubClient->Disconnect();

    spdlog::info( "Exiting." );

    return ( 0 );
}
