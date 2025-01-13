// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#include <memory>
#include <iostream>
#include <fstream>

#include "IotHubFactory.h"
#include "ProvisioningClientWrapper.h"
#include "IotHubClientWrapper.h"
#include "CustomHSM.h"
#include <iothub.h>
#include <azure_prov_client/prov_security_factory.h>
#include <azure_c_shared_utility/xlogging.h>
#include <stdarg.h>

#include <spdlog/spdlog.h>

struct IotHubFactory::IotHubFactoryImpl
{
    CustomHsm Hsm;
};

// logger instance that is set to replace the standard logger for the Azure Iot C SDk so the log output looks the same.
#if defined( __GNUC__ )
__attribute__ ((format (printf, 6, 7)))
#endif
void spdlog_logger(LOG_CATEGORY log_category, const char* file, const char* func, int line, unsigned int options, const char* format, ...)
{
    std::vector<char> msg( 1024 );
    va_list args;
    va_start( args, format );
    (void)vsnprintf( &msg[0], msg.size(), format, args );
    va_end( args );

    switch ( log_category )
    {
        case AZ_LOG_ERROR:
            spdlog::error( "File: {} Func: {} Line: {} {}", file, func, line, &msg[0] );
            break;
        default:
            spdlog::info( "{}", &msg[0] );
            break;
    }
}

IotHubFactory::IotHubFactory( const std::string& registrationId,
                              const std::string& sharedAccessSignature,
                              const std::string& certFileName,
                              const std::string& keyFileName )
    : _impl{ std::make_shared<IotHubFactoryImpl>() }
{
    xlogging_set_log_function( spdlog_logger );
    _impl->Hsm.setRegistrationId( registrationId );
    _impl->Hsm.setSas( sharedAccessSignature );

    if ( !certFileName.empty() )
    {
        std::ifstream certFile( certFileName, std::ios::binary );
        if ( certFile.is_open() )
        {
            std::vector<char> cert( ( std::istreambuf_iterator<char>( certFile ) ), std::istreambuf_iterator<char>() );
            auto result = _impl->Hsm.setCertificate( cert );
            if ( !result )
            {
                throw std::runtime_error( "Load certificate failed." );
            }
        }
    }

    if ( !keyFileName.empty() )
    {
        std::ifstream keyFile( keyFileName, std::ios::binary );
        if ( keyFile.is_open() )
        {
            std::vector<char> key( ( std::istreambuf_iterator<char>( keyFile ) ), std::istreambuf_iterator<char>() );
            auto result = _impl->Hsm.setKey( key );
            if ( !result )
            {
                throw std::runtime_error( "Load key failed." );
            }
        }
    }

    auto hsmType{ SECURE_DEVICE_TYPE_SYMMETRIC_KEY };
    if ( registrationId.empty() && sharedAccessSignature.empty() )
    {
        hsmType = SECURE_DEVICE_TYPE_X509;
    }

    prov_dev_set_symmetric_key_info( registrationId.c_str(), sharedAccessSignature.c_str() );

    if ( prov_dev_security_init( hsmType ) )
    {
        throw std::runtime_error( "Initialize security context" );
    }

    IoTHub_Init();

    spdlog::debug( "IotHubFactory" );
}

IotHubFactory::~IotHubFactory()
{
    spdlog::debug( "~IotHubFactory" );
    IoTHub_Deinit();
    prov_dev_security_deinit();
}

std::shared_ptr<IProvisioningClient> IotHubFactory::ProvisioningClient( const std::string& scope ) const
{
    return std::make_shared<ProvisioningClientWrapper>( scope );
}

std::shared_ptr<IIotHubClient> IotHubFactory::IotHubClient( const std::string& iotHubUri,
                                                            const std::string& deviceId,
                                                            const std::string& statusFile ) const
{
    return std::make_shared<IotHubClientWrapper>( iotHubUri, deviceId, statusFile );
}

std::shared_ptr<IIotHubClient> IotHubFactory::IotHubClient( const std::string& connectionString, const std::string& statusFile ) const
{
    return std::make_shared<IotHubClientWrapper>( connectionString, statusFile );
}
