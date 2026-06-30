// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#ifndef CONNECTOR_AZURESDKWRAPPER_STRINGUTILS_H_
#define CONNECTOR_AZURESDKWRAPPER_STRINGUTILS_H_

#include <string>

// Trim leading/trailing whitespace (spaces, tabs, CR/LF) from values returned
// by external SDK callbacks. This prevents malformed connection strings such as
// "HostName=<hub>.azure-devices.net\n;DeviceId=..." that fail tokenization.
inline std::string TrimWhitespace( const char* value )
{
    std::string s{ value ? value : "" };
    const char* ws = " \t\r\n";
    const auto first = s.find_first_not_of( ws );
    if ( first == std::string::npos )
    {
        return {};
    }

    const auto last = s.find_last_not_of( ws );
    return s.substr( first, last - first + 1 );
}

inline std::string TrimWhitespace( const std::string& value )
{
    return TrimWhitespace( value.c_str() );
}

#endif /* CONNECTOR_AZURESDKWRAPPER_STRINGUTILS_H_ */
