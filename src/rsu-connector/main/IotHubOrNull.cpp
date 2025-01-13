// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#include "IotHubOrNull.h"
#include <exception>
#include <memory>
#include <fstream>

struct IotHubOrNull::IotHubOrNullImpl
{
    bool Log{ true };
    std::string ScopeId;
    std::string RegistrationId;
    std::string SharedAccessSignature;
    std::string CertificateFileName;
    std::string DeviceKeyFileName;
    std::string StatusFileName;

    std::shared_ptr<IotHubFactory> Factory{ nullptr };
    std::shared_ptr<IIotHubClient> Hub{ nullptr };
    bool MethodHandlerSet{ false };
    bool DeviceTwinHandlerSet{ false };
    std::function<int( const std::string&, const std::string&, std::string& )> MethodHandler =
            []( const std::string&, const std::string&, std::string& ) -> int { return 0; };
    std::function<void( const std::string&, bool )> DeviceTwinHandler = []( const std::string&, bool ) -> void {};
};

IotHubOrNull::IotHubOrNull( std::shared_ptr<Configuration> config ) : _impl( std::make_shared<IotHubOrNullImpl>() )
{
    if ( !config )
    {
        throw std::invalid_argument( "config is required" );
    }

    try
    {
        _impl->ScopeId = config->GetStringValue( "DPS_IDSCOPE" );
        if ( _impl->ScopeId.empty() )
        {
            throw std::runtime_error( "Configuration DPS IDSCOPE is empty" );
        }
    }
    catch ( const std::exception& e )
    {
        std::string error{ "Error while reading DPS_IDSCOPE setting: " };
        error.append( e.what() );
        throw std::runtime_error( error );
    }

    try
    {
        _impl->RegistrationId = config->GetStringValue( "DPS_RegistrationId" );
    }
    catch ( const std::exception& e )
    {
        spdlog::info( "Exception while reading DPS_RegistrationId setting: {}", e.what() );
    }

    try
    {
        _impl->SharedAccessSignature = config->GetStringValue( "DPS_SharedAccessSignature" );
    }
    catch ( const std::exception& e )
    {
        spdlog::info( "Exception while reading DPS_SharedAccessSignature setting: {}", e.what() );
    }

    try
    {
        _impl->CertificateFileName = config->GetStringValue( "DPS_DeviceCertificateFile" );
    }
    catch ( const std::exception& e )
    {
        spdlog::info( "Exception while reading DPS_DeviceCertificateFile setting: {}", e.what() );
    }

    try
    {
        _impl->DeviceKeyFileName = config->GetStringValue( "DPS_DeviceKeyFile" );
    }
    catch ( const std::exception& e )
    {
        spdlog::info( "Exception while reading DPS_DeviceKeyFile setting: {}", e.what() );
    }

    if ( !_impl->RegistrationId.empty() && !_impl->SharedAccessSignature.empty() )
    {
        spdlog::info( "Using shared access signature authentification." );
    }
    else if ( !_impl->CertificateFileName.empty() && !_impl->DeviceKeyFileName.empty() )
    {
        std::ifstream keyFile( _impl->DeviceKeyFileName );
        if ( !keyFile.good() )
        {
            spdlog::error( "Could not open key file." );
            throw std::runtime_error( "Key file invalid." );
        }
        keyFile.close();

        std::ifstream certFile( _impl->CertificateFileName );
        if ( !certFile.good() )
        {
            spdlog::error( "Could not open certificate file." );
            throw std::runtime_error( "Certificate file invalid." );
        }
        certFile.close();

        spdlog::info( "Using x509 certificiate authentification." );
    }
    else
    {
        spdlog::error( "Either DPS_RegistrationId and DPS_SharedAccessSignature or DPS_DeviceCertificateFile and "
                       "DPS_DeviceKeyFile must be provided." );
        throw std::runtime_error( "No authentification method specified." );
    }

    try
    {
        _impl->StatusFileName = config->GetStringValue( "HUB_StatusFile" );
    }
    catch ( const std::exception& e )
    {
        spdlog::warn( "Exception while reading HubStatusFile: {}", e.what() );
    }

    _impl->Factory = std::make_shared<IotHubFactory>( _impl->RegistrationId,
                                                      _impl->SharedAccessSignature,
                                                      _impl->CertificateFileName,
                                                      _impl->DeviceKeyFileName );
    spdlog::debug( "IotHubOrNull" );
}

IotHubOrNull::~IotHubOrNull()
{
    spdlog::debug( "~IotHubOrNull" );
}

bool IotHubOrNull::Connect()
{
    while ( !_impl->Hub.get() )
    {
        try
        {
            std::string iotHubUri;
            std::string deviceId;
            {
                spdlog::info( "Connecting to Device Provisioning Service." );
                auto provisioningClient = _impl->Factory->ProvisioningClient( _impl->ScopeId );
                if ( provisioningClient->Register() )
                {
                    iotHubUri = provisioningClient->IotHubUri();
                    deviceId  = provisioningClient->DeviceId();
                    spdlog::info( "Register successful, hub {0} device {1}", iotHubUri, deviceId );
                }
                else
                {
                    spdlog::error( "Register to Device Provisioning Service failed." );
                    return false;
                }
                // the azure sdk is not 'reentrant' here, the provisioning client has to be destroyed before the iothub_client can be instantiated, at least with the tpm simulator.
            }
            spdlog::info( "Connecting to Iot hub." );

            _impl->Hub = _impl->Factory->IotHubClient( iotHubUri, deviceId, _impl->StatusFileName );
            if ( !_impl->Hub.get() )
            {
                spdlog::error( "IotHub client is null" );
                return false;
            }
        }
        catch ( std::exception& e )
        {
            spdlog::warn( "Exception while connecting to iothub: {}", e.what() );
            return false;
        }
    }
    _impl->Hub->SetLogTraceOption( _impl->Log );
    if ( _impl->MethodHandlerSet )
    {
        _impl->Hub->SetMethodHandler( _impl->MethodHandler );
    }
    if ( _impl->DeviceTwinHandlerSet )
    {
        _impl->Hub->SetDeviceTwinHandler( _impl->DeviceTwinHandler );
    }
    spdlog::info( "connect thread finished." );
    return true;
}

void IotHubOrNull::Disconnect()
{
    if ( _impl->Hub )
    {
        _impl->Hub->SetMethodHandler( []( auto, auto, auto ) -> int { return 0; } );
        _impl->Hub->SetDeviceTwinHandler( []( auto, auto ) {} );
        _impl->Hub.reset();
    }
}

void IotHubOrNull::SetLogTraceOption( const bool value )
{
    _impl->Log = value;
    if ( _impl->Hub.get() )
    {
        _impl->Hub->SetLogTraceOption( value );
    }
}

void IotHubOrNull::SendMessage( const MessageDataList& data,
                                const MessagePropertyList& props,
                                std::shared_ptr<IMessageLifeTimeTracker> tracker )
{
    if ( _impl->Hub.get() )
    {
        _impl->Hub->SendMessage( data, props, tracker );
    }
}

void IotHubOrNull::SendMessage( const std::string& text,
                                const MessagePropertyList& props,
                                std::shared_ptr<IMessageLifeTimeTracker> tracker )
{
    if ( _impl->Hub.get() )
    {
        _impl->Hub->SendMessage( text, props, tracker );
    }
}

bool IotHubOrNull::TriggerGetTwin()
{
    if ( _impl->Hub.get() )
    {
        return _impl->Hub->TriggerGetTwin();
    }
    else
    {
        return false;
    }
}

void IotHubOrNull::SendReportedState( const std::string& reportedStateJson )
{
    if ( _impl->Hub.get() )
    {
        _impl->Hub->SendReportedState( reportedStateJson );
    }
}

void IotHubOrNull::SetMethodHandler(
        std::function<int( const std::string&, const std::string&, std::string& )> handler )
{
    _impl->MethodHandler    = handler;
    _impl->MethodHandlerSet = true;
    if ( _impl->Hub.get() )
    {
        _impl->Hub->SetMethodHandler( handler );
    }
}

void IotHubOrNull::SetDeviceTwinHandler( std::function<void( const std::string&, bool )> handler )
{
    _impl->DeviceTwinHandler    = handler;
    _impl->DeviceTwinHandlerSet = true;
    if ( _impl->Hub.get() )
    {
        _impl->Hub->SetDeviceTwinHandler( handler );
    }
}
