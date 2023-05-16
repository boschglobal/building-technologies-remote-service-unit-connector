// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#include <string>
#include <fstream>
#include <doctest/doctest.h>
#include <spdlog/spdlog.h>

#include "UBus/UBus.cpp"
#include <libubus.h>

struct ubus_context* gUbusContext;

struct ubus_context* ubus_connect( const char* path )
{
    gUbusContext = new ubus_context;
    return gUbusContext;
}

void ubus_free( struct ubus_context* ctx )
{
    if ( gUbusContext != ctx )
    {
        throw std::runtime_error( "wrong context" );
    }
    delete ctx;
}

int ubus_lookup_id( struct ubus_context* ctx, const char* path, uint32_t* id )
{
    if ( gUbusContext != ctx )
    {
        throw std::runtime_error( "wrong context" );
    }
    return 0;
}

int ubus_invoke_fd( struct ubus_context* ctx,
                    uint32_t obj,
                    const char* method,
                    struct blob_attr* msg,
                    ubus_data_handler_t cb,
                    void* priv,
                    int timeout,
                    int fd )
{
    if ( gUbusContext != ctx )
    {
        throw std::runtime_error( "wrong context" );
    }
    return 0;
}

int blobmsg_parse( const struct blobmsg_policy* policy,
                   int policy_len,
                   struct blob_attr** tb,
                   void* data,
                   unsigned int len )
{
    return 0;
}


TEST_CASE( "ubus - can read /sbin/ip information" )
{
    Ubus ubus( "zone", "/some/path" );
    std::vector<Command::INetworkDetailSource::NetworkDetail> details;
    auto result = ubus.GetNetworkDetails( details );
    REQUIRE( result == true );
    REQUIRE( details.size() == 2 );
    REQUIRE( details[0].Ipv4Address == "192.168.1.77" );
    REQUIRE( details[1].Ipv4Address == "17.16.15.14" );
    REQUIRE( details[1].Ipv6Address == "fe80::3344:5566:7788:99aa" );
    REQUIRE( details[0].InterfaceName == "br-lan" );
    REQUIRE( details[1].InterfaceName == "tun0" );
    REQUIRE( details[0].MacAddress == "00:19:28:37:46:55" );
    REQUIRE( details[1].MacAddress.empty() );
    REQUIRE( details[0].Ipv4Gateway == "192.168.1.1" );
    REQUIRE( details[1].Ipv4Gateway.empty() );
    REQUIRE( details[0].Ipv6Gateway.empty() );
    REQUIRE( details[1].Ipv6Gateway == "fe80::1111:2222:3333:4444" );
}

struct ScopedFile
{
    std::string Name;
    ScopedFile( const std::string& name ) : Name( name )
    {
        std::ofstream file( name.c_str() );
        file.close();
    }
    ~ScopedFile() { unlink( Name.c_str() ); }
};

TEST_CASE( "ubus - invalid ip addr" )
{
    ScopedFile parseMock( "/tmp/invalid_parse_addr" );
    Ubus ubus( "zone", "/some/path" );
    std::vector<Command::INetworkDetailSource::NetworkDetail> details;
    auto result = ubus.GetNetworkDetails( details );
    REQUIRE( result == false );
    REQUIRE( details.size() == 0 );
}

TEST_CASE( "ubus - invalid ip4 route" )
{
    ScopedFile parseMock( "/tmp/invalid_parse_route4" );
    Ubus ubus( "zone", "/some/path" );
    std::vector<Command::INetworkDetailSource::NetworkDetail> details;
    auto result = ubus.GetNetworkDetails( details );
    REQUIRE( result == false );
    REQUIRE( details.size() == 0 );
}

TEST_CASE( "ubus - invalid ip6 route" )
{
    ScopedFile parseMock( "/tmp/invalid_parse_route6" );
    Ubus ubus( "zone", "/some/path" );
    std::vector<Command::INetworkDetailSource::NetworkDetail> details;
    auto result = ubus.GetNetworkDetails( details );
    REQUIRE( result == false );
    REQUIRE( details.size() == 0 );
}

TEST_CASE( "ubus - invalid type in ip addr" )
{
    ScopedFile parseMock( "/tmp/invalid_type_addr" );
    Ubus ubus( "zone", "/some/path" );
    std::vector<Command::INetworkDetailSource::NetworkDetail> details;
    auto result = ubus.GetNetworkDetails( details );
    REQUIRE( result == false );
    REQUIRE( details.size() == 0 );
}

TEST_CASE( "ubus - invalid type in ip4 route" )
{
    ScopedFile parseMock( "/tmp/invalid_type_route4" );
    Ubus ubus( "zone", "/some/path" );
    std::vector<Command::INetworkDetailSource::NetworkDetail> details;
    auto result = ubus.GetNetworkDetails( details );
    REQUIRE( result == false );
    REQUIRE( details.size() == 0 );
}

TEST_CASE( "ubus - invalid type in ip6 route" )
{
    ScopedFile parseMock( "/tmp/invalid_type_route6" );
    Ubus ubus( "zone", "/some/path" );
    std::vector<Command::INetworkDetailSource::NetworkDetail> details;
    auto result = ubus.GetNetworkDetails( details );
    REQUIRE( result == false );
    REQUIRE( details.size() == 0 );
}
