// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#include <cstddef>
#include <cstring> // avoid conflicting types for 'strchr'

#include <atomic>
#include <condition_variable>
#include <iostream>
#include <mutex>

#include "ProvisioningClientWrapper.h"

#include <azure_prov_client/prov_device_client.h>
#include <azure_prov_client/prov_security_factory.h>
#include <azure_prov_client/prov_transport_amqp_client.h>
#include <azure_prov_client/prov_transport_http_client.h>
#include <azure_c_shared_utility/shared_util_options.h>
#include <iothub.h>

#include <spdlog/spdlog.h>
#include <spdlog/fmt/bin_to_hex.h>

#include "ProvisioningClientWrapper.h"

using namespace std;

struct ProvisioningClientWrapper::ProvisioningClientWrapperImpl
{
    ProvisioningClientWrapperImpl() = default;
    ~ProvisioningClientWrapperImpl();

    void Init( const string& scope, const ProxySettings& proxy );
    bool Register();

    static void sDeviceCallback( PROV_DEVICE_RESULT result,
                                 const char* iotHubUri,
                                 const char* deviceId,
                                 void* userContextCallback )
    {
        spdlog::info( "Device callback result {} hubUrl {} device id {}", result, iotHubUri, deviceId );

        if ( !userContextCallback )
        {
            spdlog::error( "Device registration callback with NULL user context." );
            return;
        }

        ProvisioningClientWrapperImpl* ctx = static_cast<ProvisioningClientWrapperImpl*>( userContextCallback );
        if ( deviceId )
        {
            ctx->DeviceId = string( deviceId );
        }
        if ( iotHubUri )
        {
            ctx->IotHubUri = string( iotHubUri );
        }

        unique_lock<mutex> lock( ctx->Mutex );
        ctx->HasError = ( result != PROV_DEVICE_RESULT_SUCCESS ) && ( result != PROV_DEVICE_RESULT_OK );
        ctx->Finished = true;
        ctx->ResponseReceived.notify_all();
    }

    static void sStatusCallback( PROV_DEVICE_REG_STATUS status, void* userContextCallback )
    {
        spdlog::info( "Status callback {}", status );
    }

    PROV_DEVICE_HANDLE ProvisioningDeviceHandle{ NULL };
    mutex Mutex;
    condition_variable ResponseReceived;
    bool Finished{ false };
    bool HasError{ true };
    string DeviceId;
    string IotHubUri;
};


ProvisioningClientWrapper::ProvisioningClientWrapperImpl::~ProvisioningClientWrapperImpl()
{
    if ( ProvisioningDeviceHandle )
    {
        Prov_Device_Destroy( ProvisioningDeviceHandle );
    }
}

void ProvisioningClientWrapper::ProvisioningClientWrapperImpl::Init( const string& scope, const ProxySettings& proxy )
{
    if ( scope.empty() )
    {
        throw invalid_argument( "scope" );
    }

    const char global_prov_uri[] = "global.azure-devices-provisioning.net";
    auto provisioningTransport{ Prov_Device_HTTP_Protocol };
    ProvisioningDeviceHandle = Prov_Device_Create( global_prov_uri, scope.c_str(), provisioningTransport );
    if ( !ProvisioningDeviceHandle )
    {
        throw runtime_error( "Could not create provisioning client" );
    }

    if ( proxy.Enabled() )
    {
        HTTP_PROXY_OPTIONS opts = {};
        opts.host_address       = proxy.Host.c_str();
        opts.port               = proxy.Port;
        opts.username           = proxy.Username.empty() ? nullptr : proxy.Username.c_str();
        opts.password           = proxy.Password.empty() ? nullptr : proxy.Password.c_str();

        if ( Prov_Device_SetOption( ProvisioningDeviceHandle, OPTION_HTTP_PROXY, &opts ) != PROV_DEVICE_RESULT_OK )
        {
            throw runtime_error( "Setting HTTP proxy on DPS client failed." );
        }
        const char* authName = ( proxy.AuthMethod == ProxyAuthMethod::Negotiate ) ? "negotiate" : "basic";
        spdlog::info( "DPS client using proxy {}:{} (auth={})", proxy.Host, proxy.Port, authName );
    }
}

bool ProvisioningClientWrapper::ProvisioningClientWrapperImpl::Register()
{
    auto result = Prov_Device_Register_Device( ProvisioningDeviceHandle, sDeviceCallback, this, sStatusCallback, this );
    if ( result != PROV_DEVICE_RESULT_OK )
    {
        spdlog::error( "Registering device failed: {}", result );
        return false;
    }

    unique_lock<mutex> lock( Mutex );
    ResponseReceived.wait( lock, [this] { return Finished; } );

    spdlog::info( "Returning from register, error {}", HasError );
    return !HasError;
}

ProvisioningClientWrapper::ProvisioningClientWrapper( const string& scope, const ProxySettings& proxy )
    : _impl{ make_shared<ProvisioningClientWrapperImpl>() }
{
    _impl->Init( scope, proxy );
}

bool ProvisioningClientWrapper::Register()
{
    return _impl->Register();
}

const string& ProvisioningClientWrapper::DeviceId() const
{
    return _impl->DeviceId;
}

const string& ProvisioningClientWrapper::IotHubUri() const
{
    return _impl->IotHubUri;
}
