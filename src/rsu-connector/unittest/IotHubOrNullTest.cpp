// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

// Prevent IotHubOrNull.h from including IotHubFactory.h since we mock this here
#define DEVICECONNECTOR_AZURESDKWRAPPER_IOTHUBFACTORY_H_
// Prevent IotHubOrNull.h from including Configuration.h since we mock this here
#define DEVICECONNECTOR_CONFIGURATION_CONFIGURATION_H_

#include "MockIotClient.h"
#include <AzureSDKWrapper/IIotHubClient.h>
#include <AzureSDKWrapper/IProvisioningClient.h>
#include <AzureSDKWrapper/ProxySettings.h>
#include <cstdlib>
#include <map>
#include <memory>
#include <stdexcept>

struct MockConfiguration
{
    MockConfiguration() = default;
    std::string GetStringValue( const std::string& key )
    {
        auto it = Strings.find( key );
        if ( it != Strings.end() )
        {
            return it->second;
        }
        else
        {
            throw std::runtime_error( "not found" );
        }
    }
    int32_t GetIntValue( const std::string& key )
    {
        auto it = Ints.find( key );
        if ( it != Ints.end() )
        {
            return it->second;
        }
        else
        {
            throw std::runtime_error( "not found" );
        }
    }

    std::map<const std::string, std::string> Strings;
    std::map<const std::string, int32_t> Ints;
};

struct MockIotHubFactory
{
    MockIotHubFactory() = delete;
    MockIotHubFactory( const std::string& registrationId,
                       const std::string& sharedAccessSignature,
                       const std::string& certificateFileName,
                       const std::string& keyFileName,
                       const ProxySettings& proxy = {} )
    {
        DpsRegId               = registrationId;
        DpsSAS                 = sharedAccessSignature;
        DpsCertificateFileName = certificateFileName;
        DpsKeyFileName         = keyFileName;
        DpsProxy               = proxy;
    }

    virtual ~MockIotHubFactory() = default;

    std::shared_ptr<IProvisioningClient> ProvisioningClient( const std::string& scope ) const
    {
        ProvisioningClientRequested = true;
        ProvClientScope             = scope;
        return ProvClient;
    }

    std::shared_ptr<IIotHubClient> IotHubClient( const std::string& iotHubUri, const std::string& deviceId ) const
    {
        IotClientRequested = true;
        IotClientUri       = iotHubUri;
        IotClientDevId     = deviceId;
        return IotClient;
    }

    std::shared_ptr<IIotHubClient> IotHubClient( const std::string& connectionString ) const; // not used

    static std::string DpsRegId;
    static std::string DpsSAS;
    static std::string DpsCertificateFileName;
    static std::string DpsKeyFileName;
    static ProxySettings DpsProxy;
    static bool ProvisioningClientRequested;
    static bool IotClientRequested;
    static std::string ProvClientScope;
    static std::string IotClientUri;
    static std::string IotClientDevId;
    static std::shared_ptr<IProvisioningClient> ProvClient;
    static std::shared_ptr<IIotHubClient> IotClient;
};

std::string MockIotHubFactory::DpsRegId{ "" };
std::string MockIotHubFactory::DpsSAS{ "" };
std::string MockIotHubFactory::DpsCertificateFileName{ "" };
std::string MockIotHubFactory::DpsKeyFileName{ "" };
ProxySettings MockIotHubFactory::DpsProxy{};
bool MockIotHubFactory::ProvisioningClientRequested{ false };
bool MockIotHubFactory::IotClientRequested{ false };
std::string MockIotHubFactory::ProvClientScope{ "" };
std::string MockIotHubFactory::IotClientUri{ "" };
std::string MockIotHubFactory::IotClientDevId{ "" };
std::shared_ptr<IProvisioningClient> MockIotHubFactory::ProvClient{ nullptr };
std::shared_ptr<IIotHubClient> MockIotHubFactory::IotClient{ nullptr };

struct MockProvisioningClient : public IProvisioningClient
{
    virtual ~MockProvisioningClient() = default;

    virtual bool Register()
    {
        RegisterCalled++;
        if ( ThrowOnRegister )
            throw std::runtime_error( "Test throw" );
        return RegisterRetVal;
    }
    virtual const std::string& DeviceId() const { return DevId; }

    virtual const std::string& IotHubUri() const { return HubUri; }

    uint32_t RegisterCalled{ 0 };
    bool RegisterRetVal{ false };
    bool ThrowOnRegister{ false };
    std::string DevId{ "DummyDevId" };
    std::string HubUri{ "DummyHubUri" };
};


#define IotHubFactory MockIotHubFactory
#define Configuration MockConfiguration

#include <doctest/doctest.h>
#include <main/IotHubOrNull.cpp>
#include <spdlog/spdlog.h>

TEST_CASE( "iotnull - throws if no config" )
{
    CHECK_THROWS_AS( IotHubOrNull( nullptr ), std::invalid_argument );
}

TEST_CASE( "iotnull - throws if no dps scope" )
{
    auto config{ std::make_shared<MockConfiguration>() };
    CHECK_THROWS_AS( IotHubOrNull{ config }, std::runtime_error );
}

TEST_CASE( "iotnull - throws if no dps SAS or certificate" )
{
    auto config{ std::make_shared<MockConfiguration>() };
    config->Strings["DPS_IDSCOPE"] = "dummy";
    CHECK_THROWS_AS( IotHubOrNull{ config }, std::runtime_error );
}

TEST_CASE( "iotnull - throws if no dps sas" )
{
    auto config{ std::make_shared<MockConfiguration>() };
    config->Strings["DPS_IDSCOPE"]        = "dummy";
    config->Strings["DPS_RegistrationId"] = "dummy";
    CHECK_THROWS_AS( IotHubOrNull{ config }, std::runtime_error );
}

TEST_CASE( "iotnull - throws if sas but no regid" )
{
    auto config{ std::make_shared<MockConfiguration>() };
    config->Strings["DPS_IDSCOPE"]               = "dummy";
    config->Strings["DPS_SharedAccessSignature"] = "dummy";
    CHECK_THROWS_AS( IotHubOrNull{ config }, std::runtime_error );
}

TEST_CASE( "iotnull - throws if cert but no key" )
{
    auto config{ std::make_shared<MockConfiguration>() };
    config->Strings["DPS_IDSCOPE"]               = "dummy";
    config->Strings["DPS_DeviceCertificateFile"] = "dummy";
    CHECK_THROWS_AS( IotHubOrNull{ config }, std::runtime_error );
}

TEST_CASE( "iotnull - throws if key but no cert" )
{
    auto config{ std::make_shared<MockConfiguration>() };
    config->Strings["DPS_IDSCOPE"]       = "dummy";
    config->Strings["DPS_DeviceKeyFile"] = "dummy";
    CHECK_THROWS_AS( IotHubOrNull{ config }, std::runtime_error );
}

TEST_CASE( "iotnull - succeeds construction if scope and SAS settings present but does not do anything yet" )
{
    auto config{ std::make_shared<MockConfiguration>() };
    config->Strings["DPS_IDSCOPE"]                 = "dummy";
    config->Strings["DPS_RegistrationId"]          = "dummyRegId";
    config->Strings["DPS_SharedAccessSignature"]   = "dummySAS";
    MockIotHubFactory::ProvisioningClientRequested = false;
    MockIotHubFactory::IotClientRequested          = false;

    IotHubOrNull iothub{ config };
    CHECK_FALSE( MockIotHubFactory::ProvisioningClientRequested );
    CHECK_FALSE( MockIotHubFactory::IotClientRequested );
    CHECK( MockIotHubFactory::DpsRegId == "dummyRegId" );
    CHECK( MockIotHubFactory::DpsSAS == "dummySAS" );
    CHECK( MockIotHubFactory::DpsCertificateFileName.empty() );
    CHECK( MockIotHubFactory::DpsKeyFileName.empty() );
}

TEST_CASE( "iotnull - throws if key not readable" )
{
    auto config{ std::make_shared<MockConfiguration>() };
    config->Strings["DPS_IDSCOPE"]                 = "dummy";
    config->Strings["DPS_DeviceCertificateFile"]   = "/dev/null";
    config->Strings["DPS_DeviceKeyFile"]           = "dummyKey";
    MockIotHubFactory::ProvisioningClientRequested = false;
    MockIotHubFactory::IotClientRequested          = false;

    CHECK_THROWS_AS( IotHubOrNull{ config }, std::runtime_error );
}

TEST_CASE( "iotnull - throws if cert not readable" )
{
    auto config{ std::make_shared<MockConfiguration>() };
    config->Strings["DPS_IDSCOPE"]                 = "dummy";
    config->Strings["DPS_DeviceCertificateFile"]   = "dummyCert";
    config->Strings["DPS_DeviceKeyFile"]           = "/dev/null";
    MockIotHubFactory::ProvisioningClientRequested = false;
    MockIotHubFactory::IotClientRequested          = false;

    CHECK_THROWS_AS( IotHubOrNull{ config }, std::runtime_error );
}

TEST_CASE( "iotnull - succeeds construction if scope and cert settings present but does not do anything yet" )
{
    auto config{ std::make_shared<MockConfiguration>() };
    config->Strings["DPS_IDSCOPE"]                 = "dummy";
    config->Strings["DPS_DeviceCertificateFile"]   = "/dev/null";
    config->Strings["DPS_DeviceKeyFile"]           = "/dev/zero";
    MockIotHubFactory::ProvisioningClientRequested = false;
    MockIotHubFactory::IotClientRequested          = false;

    IotHubOrNull iothub{ config };
    CHECK_FALSE( MockIotHubFactory::ProvisioningClientRequested );
    CHECK_FALSE( MockIotHubFactory::IotClientRequested );
    CHECK( MockIotHubFactory::DpsRegId.empty() );
    CHECK( MockIotHubFactory::DpsSAS.empty() );
    CHECK( MockIotHubFactory::DpsCertificateFileName == "/dev/null" );
    CHECK( MockIotHubFactory::DpsKeyFileName == "/dev/zero" );
}

TEST_CASE( "iotnull - throws if invalid config combinations are specified" )
{
    auto config{ std::make_shared<MockConfiguration>() };
    config->Strings["DPS_IDSCOPE"]                 = "dummy";
    config->Strings["DPS_RegistrationId"]          = "dummyRegId";
    config->Strings["DPS_SharedAccessSignature"]   = "dummySAS";
    MockIotHubFactory::ProvisioningClientRequested = false;
    MockIotHubFactory::IotClientRequested          = false;

    SUBCASE( "scope empty" )
    {
        config->Strings["DPS_IDSCOPE"] = "";
        CHECK_THROWS_AS( IotHubOrNull{ config }, std::runtime_error );
    }
    SUBCASE( "all fields empty" )
    {
        config->Strings["DPS_RegistrationId"]        = "";
        config->Strings["DPS_SharedAccessSignature"] = "";
        config->Strings["DPS_DeviceCertificateFile"] = "";
        config->Strings["DPS_DeviceKeyFile"]         = "";
        CHECK_THROWS_AS( IotHubOrNull{ config }, std::runtime_error );
    }
    SUBCASE( "sas empty" )
    {
        config->Strings["DPS_SharedAccessSignature"] = "";
        CHECK_THROWS_AS( IotHubOrNull{ config }, std::runtime_error );
    }
}

TEST_CASE( "iotnull - requests provisioning client and iothub if Connect() is called" )
{
    auto config{ std::make_shared<MockConfiguration>() };
    config->Strings["DPS_IDSCOPE"]                 = "dummy1";
    config->Strings["DPS_RegistrationId"]          = "dummy2";
    config->Strings["DPS_SharedAccessSignature"]   = "dummy3";
    MockIotHubFactory::ProvisioningClientRequested = false;
    MockIotHubFactory::IotClientRequested          = false;
    auto prov                                      = std::make_shared<MockProvisioningClient>();
    MockIotHubFactory::ProvClient                  = prov;
    prov->RegisterRetVal                           = true;
    auto iot                                       = std::make_shared<MockIotHubClient>();
    MockIotHubFactory::IotClient                   = iot;

    IotHubOrNull iothub{ config };
    CHECK( iothub.Connect() );

    CHECK( MockIotHubFactory::ProvisioningClientRequested );
    CHECK( MockIotHubFactory::IotClientRequested );
    CHECK( prov->RegisterCalled == 1 );
    CHECK( iot->SetMethodHandlerCalled == 0 ); // no handlers should be registered yet
    CHECK( iot->SetDeviceTwinHandlerCalled == 0 );
    CHECK( MockIotHubFactory::IotClientDevId ==
           "DummyDevId" ); // check that factory was called with correct arguments for iothub
    CHECK( MockIotHubFactory::IotClientUri == "DummyHubUri" );
    iothub.Disconnect();
}

TEST_CASE( "iotnull - requests provisioning client and returns false if that fails" )
{
    auto config{ std::make_shared<MockConfiguration>() };
    config->Strings["DPS_IDSCOPE"]                 = "dummy1";
    config->Strings["DPS_RegistrationId"]          = "dummy2";
    config->Strings["DPS_SharedAccessSignature"]   = "dummy3";
    MockIotHubFactory::ProvisioningClientRequested = false;
    MockIotHubFactory::IotClientRequested          = false;
    auto prov                                      = std::make_shared<MockProvisioningClient>();
    MockIotHubFactory::ProvClient                  = prov;
    prov->RegisterRetVal                           = false;
    auto iot                                       = std::make_shared<MockIotHubClient>();
    MockIotHubFactory::IotClient                   = iot;

    IotHubOrNull iothub{ config };
    CHECK_FALSE( iothub.Connect() );

    CHECK( MockIotHubFactory::ProvisioningClientRequested );
    CHECK_FALSE( MockIotHubFactory::IotClientRequested );
    CHECK( prov->RegisterCalled == 1 );
    CHECK( iot->SetMethodHandlerCalled == 0 ); // no handlers should be registered yet
    CHECK( iot->SetDeviceTwinHandlerCalled == 0 );
}

TEST_CASE( "iotnull - requests provisioning and iot client returns false if iotclient is nullptr" )
{
    auto config{ std::make_shared<MockConfiguration>() };
    config->Strings["DPS_IDSCOPE"]                 = "dummy1";
    config->Strings["DPS_RegistrationId"]          = "dummy2";
    config->Strings["DPS_SharedAccessSignature"]   = "dummy3";
    MockIotHubFactory::ProvisioningClientRequested = false;
    MockIotHubFactory::IotClientRequested          = false;
    auto prov                                      = std::make_shared<MockProvisioningClient>();
    MockIotHubFactory::ProvClient                  = prov;
    prov->RegisterRetVal                           = true;
    std::shared_ptr<MockIotHubClient> iot{ nullptr };
    MockIotHubFactory::IotClient = iot;

    IotHubOrNull iothub{ config };
    CHECK_FALSE( iothub.Connect() );

    CHECK( MockIotHubFactory::ProvisioningClientRequested );
    CHECK( MockIotHubFactory::IotClientRequested );
    CHECK( prov->RegisterCalled == 1 );
}

TEST_CASE( "iotnull - requests provisioning client and returns false if exception is thrown" )
{
    auto config{ std::make_shared<MockConfiguration>() };
    config->Strings["DPS_IDSCOPE"]                 = "dummy1";
    config->Strings["DPS_RegistrationId"]          = "dummy2";
    config->Strings["DPS_SharedAccessSignature"]   = "dummy3";
    MockIotHubFactory::ProvisioningClientRequested = false;
    MockIotHubFactory::IotClientRequested          = false;
    auto prov                                      = std::make_shared<MockProvisioningClient>();
    MockIotHubFactory::ProvClient                  = prov;
    prov->RegisterRetVal                           = true;
    auto iot                                       = std::make_shared<MockIotHubClient>();
    MockIotHubFactory::IotClient                   = iot;
    prov->ThrowOnRegister                          = true;

    IotHubOrNull iothub{ config };
    CHECK_FALSE( iothub.Connect() );

    CHECK( MockIotHubFactory::ProvisioningClientRequested );
    CHECK_FALSE( MockIotHubFactory::IotClientRequested );
    CHECK( prov->RegisterCalled == 1 );
}


TEST_CASE( "iotnull - sets log level and methods instantly after connect" )
{
    auto config{ std::make_shared<MockConfiguration>() };
    config->Strings["DPS_IDSCOPE"]                 = "dummy1";
    config->Strings["DPS_RegistrationId"]          = "dummy2";
    config->Strings["DPS_SharedAccessSignature"]   = "dummy3";
    MockIotHubFactory::ProvisioningClientRequested = false;
    MockIotHubFactory::IotClientRequested          = false;
    auto prov                                      = std::make_shared<MockProvisioningClient>();
    MockIotHubFactory::ProvClient                  = prov;
    prov->RegisterRetVal                           = true;
    auto iot                                       = std::make_shared<MockIotHubClient>();
    MockIotHubFactory::IotClient                   = iot;

    IotHubOrNull iothub{ config };
    CHECK( iothub.Connect() );
    CHECK( iot->SetMethodHandlerCalled == 0 );
    CHECK( iot->SetDeviceTwinHandlerCalled == 0 );
    CHECK( iot->SetLogTraceOptionCalled == 1 ); // log level will always be set after Connect()
    iothub.SetLogTraceOption( true );
    iothub.SetMethodHandler( []( auto, auto, auto ) -> int { return 0; } );
    iothub.SetDeviceTwinHandler( []( auto, auto ) -> void {} );

    CHECK( iot->SetMethodHandlerCalled == 1 );
    CHECK( iot->SetDeviceTwinHandlerCalled == 1 );
    CHECK( iot->SetLogTraceOptionCalled == 2 );
}

TEST_CASE( "iotnull - sets defered log level and methods after connect" )
{
    auto config{ std::make_shared<MockConfiguration>() };
    config->Strings["DPS_IDSCOPE"]                 = "dummy1";
    config->Strings["DPS_RegistrationId"]          = "dummy2";
    config->Strings["DPS_SharedAccessSignature"]   = "dummy3";
    MockIotHubFactory::ProvisioningClientRequested = false;
    MockIotHubFactory::IotClientRequested          = false;
    auto prov                                      = std::make_shared<MockProvisioningClient>();
    MockIotHubFactory::ProvClient                  = prov;
    prov->RegisterRetVal                           = true;
    auto iot                                       = std::make_shared<MockIotHubClient>();
    MockIotHubFactory::IotClient                   = iot;

    IotHubOrNull iothub{ config };
    iothub.SetLogTraceOption( true );
    iothub.SetMethodHandler( []( auto, auto, auto ) -> int { return 0; } );
    iothub.SetDeviceTwinHandler( []( auto, auto ) -> void {} );
    CHECK( iothub.Connect() );

    CHECK( MockIotHubFactory::ProvisioningClientRequested );
    CHECK( MockIotHubFactory::IotClientRequested );
    CHECK( prov->RegisterCalled == 1 );
    CHECK( iot->SetMethodHandlerCalled == 1 );
    CHECK( iot->SetDeviceTwinHandlerCalled == 1 );
    CHECK( iot->SetLogTraceOptionCalled == 1 );
}

TEST_CASE(
        "iotnull - ignores sendmessage and device twin functionality before connect but forwards them after connect" )
{
    auto config{ std::make_shared<MockConfiguration>() };
    config->Strings["DPS_IDSCOPE"]                 = "dummy1";
    config->Strings["DPS_RegistrationId"]          = "dummy2";
    config->Strings["DPS_SharedAccessSignature"]   = "dummy3";
    MockIotHubFactory::ProvisioningClientRequested = false;
    MockIotHubFactory::IotClientRequested          = false;
    auto prov                                      = std::make_shared<MockProvisioningClient>();
    MockIotHubFactory::ProvClient                  = prov;
    prov->RegisterRetVal                           = true;
    auto iot                                       = std::make_shared<MockIotHubClient>();
    MockIotHubFactory::IotClient                   = iot;

    IotHubOrNull iothub{ config };
    iothub.SendMessage( "Test message", {}, nullptr );
    iothub.SendMessage( std::vector<uint8_t>{ 1, 2, 3, 4, 5 }, {}, nullptr );
    iothub.TriggerGetTwin();
    iothub.SendReportedState( "{}" );

    CHECK( iothub.Connect() );

    CHECK( iot->SendMessageBinaryCalled.empty() );
    CHECK( iot->SendMessageStringCalled.empty() );
    CHECK( iot->SendReportedStateCalled == 0 );
    CHECK( iot->GetDeviceTwinCalled == 0 );

    iothub.SendMessage( "Test message", {}, nullptr );
    iothub.SendMessage( std::vector<uint8_t>{ 1, 2, 3, 4, 5 }, {}, nullptr );
    iothub.TriggerGetTwin();
    iothub.SendReportedState( "{}" );

    CHECK( iot->SendMessageBinaryCalled.size() == 1 );
    CHECK( iot->SendMessageStringCalled.size() == 1 );
    CHECK( iot->SendReportedStateCalled == 1 );
    CHECK( iot->GetDeviceTwinCalled == 1 );
}

TEST_CASE( "iotnull - proxy fully configured is forwarded to factory" )
{
    auto config{ std::make_shared<MockConfiguration>() };
    config->Strings["DPS_IDSCOPE"]               = "dummy";
    config->Strings["DPS_RegistrationId"]        = "dummyRegId";
    config->Strings["DPS_SharedAccessSignature"] = "dummySAS";
    config->Strings["Proxy_Host"]                = "proxy.example.com";
    config->Ints["Proxy_Port"]                   = 8080;
    config->Strings["Proxy_Username"]            = "alice";
    config->Strings["Proxy_Password"]            = "s3cret";
    MockIotHubFactory::DpsProxy                  = ProxySettings{};

    IotHubOrNull iothub{ config };
    CHECK( MockIotHubFactory::DpsProxy.Host == "proxy.example.com" );
    CHECK( MockIotHubFactory::DpsProxy.Port == 8080 );
    CHECK( MockIotHubFactory::DpsProxy.Username == "alice" );
    CHECK( MockIotHubFactory::DpsProxy.Password == "s3cret" );
    CHECK( MockIotHubFactory::DpsProxy.Enabled() );
}

TEST_CASE( "iotnull - proxy host and port without credentials is forwarded with empty auth" )
{
    auto config{ std::make_shared<MockConfiguration>() };
    config->Strings["DPS_IDSCOPE"]               = "dummy";
    config->Strings["DPS_RegistrationId"]        = "dummyRegId";
    config->Strings["DPS_SharedAccessSignature"] = "dummySAS";
    config->Strings["Proxy_Host"]                = "proxy.example.com";
    config->Ints["Proxy_Port"]                   = 3128;
    MockIotHubFactory::DpsProxy                  = ProxySettings{};

    IotHubOrNull iothub{ config };
    CHECK( MockIotHubFactory::DpsProxy.Host == "proxy.example.com" );
    CHECK( MockIotHubFactory::DpsProxy.Port == 3128 );
    CHECK( MockIotHubFactory::DpsProxy.Username.empty() );
    CHECK( MockIotHubFactory::DpsProxy.Password.empty() );
    CHECK( MockIotHubFactory::DpsProxy.Enabled() );
}

TEST_CASE( "iotnull - absent proxy keys leave proxy disabled" )
{
    auto config{ std::make_shared<MockConfiguration>() };
    config->Strings["DPS_IDSCOPE"]               = "dummy";
    config->Strings["DPS_RegistrationId"]        = "dummyRegId";
    config->Strings["DPS_SharedAccessSignature"] = "dummySAS";
    MockIotHubFactory::DpsProxy                  = ProxySettings{ "stale", 9999, "x", "y" };

    IotHubOrNull iothub{ config };
    CHECK_FALSE( MockIotHubFactory::DpsProxy.Enabled() );
    CHECK( MockIotHubFactory::DpsProxy.Host.empty() );
    CHECK( MockIotHubFactory::DpsProxy.Port == 0 );
}

TEST_CASE( "iotnull - proxy host without valid port stays disabled" )
{
    auto config{ std::make_shared<MockConfiguration>() };
    config->Strings["DPS_IDSCOPE"]               = "dummy";
    config->Strings["DPS_RegistrationId"]        = "dummyRegId";
    config->Strings["DPS_SharedAccessSignature"] = "dummySAS";
    config->Strings["Proxy_Host"]                = "proxy.example.com";
    config->Ints["Proxy_Port"]                   = 0;
    MockIotHubFactory::DpsProxy                  = ProxySettings{};

    IotHubOrNull iothub{ config };
    CHECK( MockIotHubFactory::DpsProxy.Host == "proxy.example.com" );
    CHECK( MockIotHubFactory::DpsProxy.Port == 0 );
    CHECK_FALSE( MockIotHubFactory::DpsProxy.Enabled() );
}

TEST_CASE( "iotnull - proxy with negotiate auth and keytab exports KRB5 env vars" )
{
    unsetenv( "KRB5_CLIENT_KTNAME" );
    unsetenv( "KRB5CCNAME" );

    auto config{ std::make_shared<MockConfiguration>() };
    config->Strings["DPS_IDSCOPE"]               = "dummy";
    config->Strings["DPS_RegistrationId"]        = "dummyRegId";
    config->Strings["DPS_SharedAccessSignature"] = "dummySAS";
    config->Strings["Proxy_Host"]                = "proxy.example.com";
    config->Ints["Proxy_Port"]                   = 8080;
    config->Strings["Proxy_AuthMethod"]          = "negotiate";
    config->Strings["Proxy_Keytab"]              = "/etc/rsu-connector/rsu.keytab";
    MockIotHubFactory::DpsProxy                  = ProxySettings{};

    IotHubOrNull iothub{ config };

    CHECK( MockIotHubFactory::DpsProxy.Enabled() );
    CHECK( MockIotHubFactory::DpsProxy.AuthMethod == ProxyAuthMethod::Negotiate );
    CHECK( MockIotHubFactory::DpsProxy.Keytab == "/etc/rsu-connector/rsu.keytab" );

    const char* ktname = getenv( "KRB5_CLIENT_KTNAME" );
    REQUIRE( ktname != nullptr );
    CHECK( std::string( ktname ) == "/etc/rsu-connector/rsu.keytab" );

    const char* ccname = getenv( "KRB5CCNAME" );
    REQUIRE( ccname != nullptr );
    CHECK( std::string( ccname ) == "FILE:/tmp/rsu-connector-krb5cc" );

    unsetenv( "KRB5_CLIENT_KTNAME" );
    unsetenv( "KRB5CCNAME" );
}

TEST_CASE( "iotnull - proxy with negotiate auth and no keytab leaves env vars unset" )
{
    unsetenv( "KRB5_CLIENT_KTNAME" );
    unsetenv( "KRB5CCNAME" );

    auto config{ std::make_shared<MockConfiguration>() };
    config->Strings["DPS_IDSCOPE"]               = "dummy";
    config->Strings["DPS_RegistrationId"]        = "dummyRegId";
    config->Strings["DPS_SharedAccessSignature"] = "dummySAS";
    config->Strings["Proxy_Host"]                = "proxy.example.com";
    config->Ints["Proxy_Port"]                   = 8080;
    config->Strings["Proxy_AuthMethod"]          = "Negotiate";
    MockIotHubFactory::DpsProxy                  = ProxySettings{};

    IotHubOrNull iothub{ config };

    CHECK( MockIotHubFactory::DpsProxy.AuthMethod == ProxyAuthMethod::Negotiate );
    CHECK( MockIotHubFactory::DpsProxy.Keytab.empty() );
    CHECK( getenv( "KRB5_CLIENT_KTNAME" ) == nullptr );
    CHECK( getenv( "KRB5CCNAME" ) == nullptr );
}

TEST_CASE( "iotnull - proxy with basic auth (default) leaves env vars unset" )
{
    unsetenv( "KRB5_CLIENT_KTNAME" );
    unsetenv( "KRB5CCNAME" );

    auto config{ std::make_shared<MockConfiguration>() };
    config->Strings["DPS_IDSCOPE"]               = "dummy";
    config->Strings["DPS_RegistrationId"]        = "dummyRegId";
    config->Strings["DPS_SharedAccessSignature"] = "dummySAS";
    config->Strings["Proxy_Host"]                = "proxy.example.com";
    config->Ints["Proxy_Port"]                   = 8080;
    config->Strings["Proxy_Username"]            = "alice";
    config->Strings["Proxy_Password"]            = "s3cret";
    config->Strings["Proxy_Keytab"]              = "/etc/rsu-connector/rsu.keytab"; // ignored when method=basic
    MockIotHubFactory::DpsProxy                  = ProxySettings{};

    IotHubOrNull iothub{ config };

    CHECK( MockIotHubFactory::DpsProxy.AuthMethod == ProxyAuthMethod::Basic );
    CHECK( getenv( "KRB5_CLIENT_KTNAME" ) == nullptr );
    CHECK( getenv( "KRB5CCNAME" ) == nullptr );
}
