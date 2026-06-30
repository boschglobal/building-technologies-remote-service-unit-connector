// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#include <AzureSDKWrapper/StringUtils.h>
#include <doctest/doctest.h>

TEST_CASE( "StringUtils - trims trailing newline from DPS hub URI" )
{
    CHECK( "prod-weu-rsf-hub.azure-devices.net" == TrimWhitespace( "prod-weu-rsf-hub.azure-devices.net\n" ) );
}

TEST_CASE( "StringUtils - trims whitespace around DPS device id" )
{
    CHECK( "00:01:49:FF:00:06" == TrimWhitespace( "\t00:01:49:FF:00:06\r\n" ) );
}

TEST_CASE( "StringUtils - returns empty for whitespace-only input" )
{
    CHECK( TrimWhitespace( " \t\r\n" ).empty() );
}
