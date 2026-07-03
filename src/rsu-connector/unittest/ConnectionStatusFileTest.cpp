// Copyright (c) 2026 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#include <experimental/filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <AzureSDKWrapper/ConnectionStatusFile.h>
#include <doctest/doctest.h>

namespace fs = std::experimental::filesystem;

static std::string ReadFile( const std::string& path )
{
    std::ifstream file( path );
    std::stringstream content;
    content << file.rdbuf();
    return content.str();
}

struct StatusDirGuard
{
    const fs::path Dir{ fs::path( "connection_status_file_test" ) };
    ~StatusDirGuard()
    {
        std::error_code ec;
        fs::remove_all( Dir, ec );
    }
};

TEST_CASE( "statusfile - empty path writes nothing" )
{
    StatusDirGuard guard;
    WriteConnectionStatusFile( "", "OK" );
    CHECK( !fs::exists( guard.Dir ) );
}

TEST_CASE( "statusfile - writes status as json" )
{
    StatusDirGuard guard;
    const std::string path = ( guard.Dir / "status.json" ).string();

    WriteConnectionStatusFile( path, "OK" );

    CHECK( ReadFile( path ) == "{ \"status\": \"OK\" }\n" );
}

TEST_CASE( "statusfile - creates missing directories" )
{
    StatusDirGuard guard;
    const std::string path = ( guard.Dir / "sub" / "dir" / "status.json" ).string();

    WriteConnectionStatusFile( path, "NO_NETWORK" );

    CHECK( ReadFile( path ) == "{ \"status\": \"NO_NETWORK\" }\n" );
}

TEST_CASE( "statusfile - replaces existing file and leaves no temp file" )
{
    StatusDirGuard guard;
    const std::string path = ( guard.Dir / "status.json" ).string();

    WriteConnectionStatusFile( path, "OK" );
    WriteConnectionStatusFile( path, "COMMUNICATION_ERROR" );

    CHECK( ReadFile( path ) == "{ \"status\": \"COMMUNICATION_ERROR\" }\n" );
    CHECK( !fs::exists( path + ".tmp" ) );
}
