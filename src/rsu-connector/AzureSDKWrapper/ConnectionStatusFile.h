// Copyright (c) 2026 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#ifndef CONNECTOR_AZURESDKWRAPPER_CONNECTIONSTATUSFILE_H_
#define CONNECTOR_AZURESDKWRAPPER_CONNECTIONSTATUSFILE_H_

#include <cstdio>
#include <experimental/filesystem>
#include <fstream>
#include <string>
#include <system_error>

#include <spdlog/spdlog.h>

/// @brief Write the IoT Hub connection status to the given file as JSON, e.g. { "status": "OK" }.
///
/// Missing directories in the path are created. An empty path disables the file (no-op).
/// @param path File the status is written to.
/// @param status Status text, one of the IOTHUB_CLIENT_CONNECTION_STATUS_REASON names or UNKNOWN.
inline void WriteConnectionStatusFile( const std::string& path, const char* status )
{
    if ( path.empty() )
    {
        return;
    }
    namespace fs             = std::experimental::filesystem;
    const fs::path parentDir = fs::path( path ).parent_path();
    if ( !parentDir.empty() )
    {
        std::error_code ec;
        fs::create_directories( parentDir, ec );
        // on failure the ofstream open below fails and warns
    }
    // Write to a temporary file and rename so a reader never sees a partially written file.
    const std::string tempPath = path + ".tmp";
    std::ofstream file( tempPath );
    if ( !file )
    {
        spdlog::warn( "Status file {} cannot be created", tempPath );
        return;
    }
    file << "{ \"status\": \"" << status << "\" }\n";
    file.close();
    if ( !file || std::rename( tempPath.c_str(), path.c_str() ) != 0 )
    {
        spdlog::warn( "Status file {} cannot be written", path );
    }
}

#endif // CONNECTOR_AZURESDKWRAPPER_CONNECTIONSTATUSFILE_H_
