// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#include <UBus/UBus.h>
#include <cstring>
#include <fstream>
#include <regex>
#include <experimental/filesystem>
#include <spdlog/spdlog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>

#define RAPIDJSON_HAS_STDSTRING 1
#define RAPIDJSON_HAS_CXX11_RANGE_FOR 1
#include <rapidjson/rapidjson.h>
#include <rapidjson/document.h>

extern "C" {
#include <libubox/blobmsg.h>
#include <libubus.h>
#include <uci.h>
};

struct Ubus::UbusImpl
{
    UbusImpl()  = default;
    ~UbusImpl() = default;

    std::string FirewallZone;
    std::string Name;
    std::string Hostname;
    std::string Type;
    std::string Serial;
    std::string Mac;
    std::string FirmwareVersion;
    std::string OpenVpnLogPath;
    bool ReadProperties();
    bool DeleteRedirect( const std::string& name );
    bool ClearRedirect();
    bool AddRedirect( const Command::IPortForwardingActor::PortForwardingConfiguration& config );
    bool ReloadFirewall();
    ConnectionStatusVPN StartVPN( const std::string& name, std::string& ipAddress );
    ConnectionStatusVPN StopVPN( const std::string& name );
    bool ConfigureVPN( const std::string& name, const std::string& config );
    bool GetNetworkDetails( std::vector<NetworkDetail>& details );
    bool GetVPNConfigurations( std::vector<VPNConfiguration>& config );
    bool GetPortForwardings( std::vector<PortForwardingConfiguration>& config );

    int SocketFD{ -1 };
};

Ubus::Ubus( const std::string& firewallZone, const std::string& openVpnLogPath )
    : _impl( std::make_shared<Ubus::UbusImpl>() )
{
    _impl->FirewallZone   = firewallZone;
    _impl->OpenVpnLogPath = openVpnLogPath;
    if ( !_impl->ReadProperties() )
    {
        throw std::runtime_error( "Could not read properties from ubus" );
    }
}

Ubus::~Ubus() {}

bool Ubus::UbusImpl::DeleteRedirect( const std::string& name )
{
    struct uci_context* uci_ctx;
    struct uci_package* uci_firewall;
    struct uci_section* s;
    struct uci_element* e;
    struct uci_ptr ptr = {};
    const char* uci_name;
    bool valid = false;

    uci_ctx = uci_alloc_context();
    uci_load( uci_ctx, "firewall", &uci_firewall );

    /* Check if redirect name is deleteable */
    uci_foreach_element( &uci_firewall->sections, e )
    {
        s = uci_to_section( e );

        if ( strcmp( s->type, "redirect" ) )
            continue;

        uci_name = uci_lookup_option_string( uci_ctx, s, "name" );
        if ( !uci_name )
            continue;

        if ( name == uci_name )
            valid = true;
    }

    if ( !valid )
    {
        spdlog::info( "Redirect name not found ({}).", name );
        uci_free_context( uci_ctx );
        return false;
    }

    ptr.p = uci_firewall;
    ptr.s = s;
    ptr.o = NULL;
    uci_delete( uci_ctx, &ptr );

    uci_save( uci_ctx, uci_firewall );
    uci_commit( uci_ctx, &uci_firewall, false );

    uci_free_context( uci_ctx );

    return true;
}

bool Ubus::UbusImpl::ClearRedirect()
{
    struct uci_context* uci_ctx;
    struct uci_package* uci_firewall;
    struct uci_element* element;
    struct uci_element* tmp;

    uci_ctx = uci_alloc_context();
    uci_load( uci_ctx, "firewall", &uci_firewall );

    /* Delete all redirects */
    uci_foreach_element_safe( &uci_firewall->sections, tmp, element )
    {
        struct uci_section* s = uci_to_section( element );

        if ( strcmp( s->type, "redirect" ) )
            continue;

        struct uci_ptr p;
        memset( &p, 0, sizeof( p ) );

        /* This name variable will be changed by uci_lookup_ptr !!! */
        char name[64];
        snprintf( name, sizeof( name ), "%s.%s=%s", "firewall", (char*)s->e.name, "redirect" );

        int ret;
        ret = uci_lookup_ptr( uci_ctx, &p, name, false );

        if ( ret == UCI_OK )
            uci_delete( uci_ctx, &p );

        if ( ret != UCI_OK )
            spdlog::info( "Unable to delete redirect section {}", name );
    }

    uci_save( uci_ctx, uci_firewall );
    uci_commit( uci_ctx, &uci_firewall, false );

    uci_free_context( uci_ctx );

    return true;
}

bool Ubus::UbusImpl::AddRedirect( const Command::IPortForwardingActor::PortForwardingConfiguration& config )
{
    struct uci_context* uci_ctx;
    struct uci_package* uci_firewall;
    struct uci_section* s;
    struct uci_element* e;
    struct uci_ptr ptr = {};
    const char* uci_name;
    bool valid = true;

    uci_ctx = uci_alloc_context();
    uci_load( uci_ctx, "firewall", &uci_firewall );

    /* Check if redirect name is already set */
    uci_foreach_element( &uci_firewall->sections, e )
    {
        s = uci_to_section( e );

        if ( strcmp( s->type, "redirect" ) )
            continue;

        uci_name = uci_lookup_option_string( uci_ctx, s, "name" );
        if ( !uci_name )
            continue;

        if ( config.Name == uci_name )
            valid = false;
    }

    if ( !valid )
    {
        spdlog::info( "Redirect name already in use ({})", config.Name );
        uci_free_context( uci_ctx );
        return false;
    }

    uci_add_section( uci_ctx, uci_firewall, "redirect", &s );

    ptr.p = s->package;
    ptr.s = s;

    /* Default options for DNAT redirect on bosinet */
    ptr.o      = NULL;
    ptr.option = "src";
    ptr.value  = FirewallZone.c_str();
    uci_set( uci_ctx, &ptr );

    ptr.o      = NULL;
    ptr.option = "dest";
    ptr.value  = "lan";
    uci_set( uci_ctx, &ptr );

    ptr.o      = NULL;
    ptr.option = "target";
    ptr.value  = "DNAT";
    uci_set( uci_ctx, &ptr );

    ptr.o      = NULL;
    ptr.option = "family";
    ptr.value  = "ipv4";
    uci_set( uci_ctx, &ptr );

    ptr.o      = NULL;
    ptr.option = "proto";
    ptr.value  = "tcp udp";
    uci_set( uci_ctx, &ptr );

    /* Received options form the IoT-Hub */
    /* @todo validate redirect configuration from IoT-Hub */
    ptr.o      = NULL;
    ptr.option = "name";
    ptr.value  = config.Name.c_str();
    uci_set( uci_ctx, &ptr );

    /*
	 * The option src_ip is not used because openwrt uses a firewall
	 * zone concept. We should use instead the option src
	 */
    //	ptr.o = NULL;
    //	ptr.option = "src_ip";
    //	ptr.value  = blobmsg_get_string(redirect[REDIRECT_SRCIP]);
    //	uci_set(uci_ctx, &ptr);

    ptr.o      = NULL;
    ptr.option = "src_dport";
    ptr.value  = config.SourcePort.c_str();
    uci_set( uci_ctx, &ptr );

    ptr.o      = NULL;
    ptr.option = "dest_ip";
    ptr.value  = config.DestinationIP.c_str();
    uci_set( uci_ctx, &ptr );

    ptr.o      = NULL;
    ptr.option = "dest_port";
    ptr.value  = config.DestinationPort.c_str();
    uci_set( uci_ctx, &ptr );

    uci_save( uci_ctx, uci_firewall );
    uci_commit( uci_ctx, &uci_firewall, true );

    uci_free_context( uci_ctx );

    return true;
}


enum
{
    SYSTEM_BOARD_MODEL,
    SYSTEM_BOARD_HOSTNAME,
    SYSTEM_BOARD_SERIAL,
    SYSTEM_BOARD_MAC,
    SYSTEM_BOARD_RELEASE,
    __SYSTEM_BOARD_MAX
};

static const struct blobmsg_policy system_board_policy[__SYSTEM_BOARD_MAX] = {
        [SYSTEM_BOARD_MODEL]    = { .name = "model", .type = BLOBMSG_TYPE_STRING },
        [SYSTEM_BOARD_HOSTNAME] = { .name = "hostname", .type = BLOBMSG_TYPE_STRING },
        [SYSTEM_BOARD_SERIAL]   = { .name = "serial", .type = BLOBMSG_TYPE_STRING },
        [SYSTEM_BOARD_MAC]      = { .name = "mac", .type = BLOBMSG_TYPE_STRING },
        [SYSTEM_BOARD_RELEASE]  = { .name = "release", .type = BLOBMSG_TYPE_TABLE },
};

static struct blob_attr* system_board = nullptr;

static void ubus_system_board_cb( struct ubus_request* req, int type, struct blob_attr* msg )
{
    spdlog::info( "Ubus system board callback invoked" );
    system_board = blob_memdup( msg );
}

bool Ubus::UbusImpl::ReadProperties()
{
    uint32_t id;
    struct ubus_context* ctx = ubus_connect( NULL );

    if ( !ctx )
    {
        spdlog::info( "Unable to connect to ubus backend" );
        return false;
    }

    if ( ubus_lookup_id( ctx, "system", &id ) )
    {
        spdlog::info( "Unable to connect to ubus path 'system'" );
        ubus_free( ctx );
        return false;
    }

    if ( ubus_invoke( ctx, id, "board", NULL, ubus_system_board_cb, NULL, 500 ) )
    {
        spdlog::info( "Unable to call ubus method 'board' from path 'system'" );
        ubus_free( ctx );
        return false;
    }

    if ( system_board )
    {
        struct blob_attr* tb[__SYSTEM_BOARD_MAX];
        blobmsg_parse( system_board_policy,
                       __SYSTEM_BOARD_MAX,
                       tb,
                       blobmsg_data( system_board ),
                       blobmsg_len( system_board ) );

        if ( blobmsg_data( tb[SYSTEM_BOARD_HOSTNAME] ) )
        {
            Hostname = blobmsg_get_string( tb[SYSTEM_BOARD_HOSTNAME] );
        }
        if ( blobmsg_data( tb[SYSTEM_BOARD_MODEL] ) )
        {
            Type = blobmsg_get_string( tb[SYSTEM_BOARD_MODEL] );
        }
        if ( blobmsg_data( tb[SYSTEM_BOARD_MAC] ) )
        {
            Mac = blobmsg_get_string( tb[SYSTEM_BOARD_MAC] );
        }
        if ( blobmsg_data( tb[SYSTEM_BOARD_SERIAL] ) )
        {
            Serial = blobmsg_get_string( tb[SYSTEM_BOARD_SERIAL] );
        }

        if ( tb[SYSTEM_BOARD_RELEASE] )
        {
            struct blob_attr* release;
            int i;
            blobmsg_for_each_attr( release, tb[SYSTEM_BOARD_RELEASE], i )
            {
                if ( !strcmp( "version", blobmsg_name( release ) ) )
                {
                    FirmwareVersion = blobmsg_get_string( release );
                }
            }
        }
        free( system_board );
    }
    return true;
}

bool Ubus::UbusImpl::ReloadFirewall()
{
    uint32_t id;
    struct blob_buf data = {};
    void* a;

    struct ubus_context* ctx = ubus_connect( NULL );

    if ( !ctx )
    {
        spdlog::info( "Unable to connect to ubus backend" );
        return false;
    }


    if ( ubus_lookup_id( ctx, "service", &id ) )
    {
        spdlog::info( "Unable to connect to ubus path 'service'" );
        ubus_free( ctx );
        return false;
    }

    blob_buf_init( &data, 0 );
    blobmsg_add_string( &data, "type", "config.change" );
    a = blobmsg_open_table( &data, "data" );
    blobmsg_add_string( &data, "package", "firewall" );
    blobmsg_close_table( &data, a );

    ubus_invoke( ctx, id, "event", data.head, NULL, NULL, 500 );
    spdlog::info( "Trigger firewall reload handling" );
    blob_buf_free( &data );
    ubus_free( ctx );
    return true;
}

// Run a command and return its output and its error code.
static std::pair<std::string, int> runCmdWithOutput( const std::string& command )
{
    std::vector<char> buffer( 1024 );
    std::string output;
    auto pipe = popen( command.c_str(), "r" );
    if ( !pipe )
    {
        spdlog::error( "Pipe open for command {} failed: {}", command, errno );
        return std::pair<std::string, int>( output, 0 );
    }
    size_t bytesRead{ 0 };
    while ( ( bytesRead = fread( buffer.data(), 1U, buffer.size(), pipe ) ) > 0 )
    {
        output += std::string_view( &buffer[0], bytesRead );
    }
    auto error = pclose( pipe );
    if ( error != 0 )
    {
        spdlog::error( "Command {} return error code {}", command, error );
        output = "";
    }
    return std::pair<std::string, int>( output, error );
}

static bool IsVpnRunning( const std::string& name )
{
    auto [currentStatus, error] = runCmdWithOutput( fmt::format( "/etc/init.d/openvpn status {}", name ) );
    if ( !error && currentStatus.find( "running" ) != std::string::npos )
    {
        return true;
    }
    else
    {
        return false;
    }
}

// Reads a openvpn config file and extracts one line via a regex pattern.
// Returns a smatch instance that includes the captures from the regex.
static std::smatch ReadConfigFileOption( const std::string& filename, const std::regex& configPattern )
{
    std::ifstream configFile( filename.c_str() );
    std::string line;
    std::smatch matches;
    while ( configFile )
    {
        std::getline( configFile, line );
        if ( regex_search( line, matches, configPattern ) )
        {
            break;
        }
    }
    configFile.close();
    return matches;
}

Command::IVPNActor::ConnectionStatusVPN Ubus::UbusImpl::StartVPN( const std::string& name, std::string& ipAddress )
{
    std::string filename{ "/etc/openvpn/" };
    filename.append( name );
    filename.append( ".conf" );

    struct stat buffer;
    if ( stat( filename.c_str(), &buffer ) != 0 )
    {
        spdlog::error( "Configuration does not exists: {}", filename );
        return NoConfiguration;
    }

    std::regex logPattern( "^log +([^ ]+)", std::regex::ECMAScript );
    auto logMatch = ReadConfigFileOption( filename, logPattern );
    if ( logMatch.size() != 2 )
    {
        spdlog::error( "Could not find log file name within {}. Please make sure the configuration contains a 'log "
                       "<filename>' line with <filename> being unique for this configuration.",
                       filename );
        return Error;
    }

    auto logFilePath = logMatch[1].str();

    spdlog::info( "Checking for VPN {} already started.", name );
    if ( IsVpnRunning( name ) )
    {
        spdlog::warn( "Configuration already connected: {}", name );

        // Find the network device that is referred to in the configuration to find out its IP
        std::regex devPattern( "^dev +([^ ]+)", std::regex::ECMAScript );
        auto devMatch = ReadConfigFileOption( filename, devPattern );
        if ( devMatch.size() == 2 )
        {
            const auto deviceName = devMatch[1].str();
            std::vector<Command::INetworkDetailSource::NetworkDetail> details;
            (void)GetNetworkDetails( details );
            auto network = std::find_if( details.begin(), details.end(), [deviceName]( auto& element ) {
                return element.InterfaceName == deviceName;
            } );
            if ( network != details.end() )
            {
                ipAddress = network->Ipv4Address;
                return AlreadyRunning;
            }
        }
        else
        {
            spdlog::error( "Could not find device name within {}. Please make sure the configuration contains a 'dev "
                           "<name>' line with <name> being unique for this configuration.",
                           filename );
            return Error;
        }
    }

    off_t oldpos = 0;
    spdlog::info( "Checking for log file presence." );
    auto fd = open( logFilePath.c_str(), O_RDONLY );
    if ( fd < 0 )
    {
        spdlog::warn( "Could not open {}", logFilePath );
    }
    else
    {
        (void)lseek( fd, 0, SEEK_END );
        oldpos = lseek( fd, 0, SEEK_SET );
        close( fd );
    }

    spdlog::info( "Starting openvpn interface." );
    std::string command{ "/etc/init.d/openvpn start " };
    command.append( name );
    auto [output, error] = runCmdWithOutput( command.c_str() );
    if ( error )
    {
        return Error;
    }
    (void)output;

    auto startTime     = std::chrono::steady_clock::now();
    const auto endTime = startTime + std::chrono::seconds( 15 );

    std::string pattern{ "openvpn-hotplug up " };
    pattern.append( name );
    pattern.append( " [^ ]+ [0-9]+ [0-9]+ ([0-9.]+) " );
    std::regex startLine( pattern, std::regex_constants::ECMAScript );

    spdlog::info( "Waiting for connection in log." );
    std::string text;
    bool gotStart{ false };
    while ( std::chrono::steady_clock::now() < endTime )
    {
        auto fd = open( logFilePath.c_str(), O_RDONLY );
        if ( fd < 0 )
        {
            spdlog::warn( "Could not open {}, will retry", logFilePath );
            usleep( 100000 );
            continue;
        }
        (void)lseek( fd, oldpos, SEEK_SET );
        std::vector<char> buffer( 1024 );
        auto bytesRead = read( fd, &buffer[0], buffer.size() );
        close( fd );
        if ( bytesRead == 0 )
        {
            usleep( 100000 );
            continue;
        }
        if ( bytesRead < 0 )
        {
            break;
        }
        text.append( &buffer[0], &buffer[bytesRead] );
        oldpos += bytesRead;
        std::smatch matches;
        if ( regex_search( text, matches, startLine ) && matches.size() == 2 )
        {
            ipAddress = matches[1].str();
            spdlog::info( "Found IP in log: {}", ipAddress );
            gotStart = true;
            break;
        }
    }
    if ( !gotStart )
    {
        return Error;
    }

    return Established;
}

Command::IVPNActor::ConnectionStatusVPN Ubus::UbusImpl::StopVPN( const std::string& name )
{
    std::string filename{ "/etc/openvpn/" };
    filename.append( name );
    filename.append( ".conf" );

    struct stat buffer;
    if ( stat( filename.c_str(), &buffer ) != 0 )
    {
        spdlog::error( "Configuration does not exists: {}", filename );
        return NoConfiguration;
    }

    if ( !IsVpnRunning( name ) )
    {
        spdlog::warn( "Configuration {} is not connected, will try to stop anyway.", name );
    }

    std::string command{ "/etc/init.d/openvpn stop " };
    command.append( name );
    auto [output, error] = runCmdWithOutput( command.c_str() );
    if ( error )
    {
        return Error;
    }
    (void)output;

    auto startTime     = std::chrono::steady_clock::now();
    const auto endTime = startTime + std::chrono::seconds( 10 );

    bool gotStop{ false };
    while ( std::chrono::steady_clock::now() < endTime )
    {
        if ( !IsVpnRunning( name ) )
        {
            gotStop = true;
            break;
        }
        usleep( 100000 );
    }
    return gotStop ? Disconnected : Error;
}

bool Ubus::UbusImpl::ConfigureVPN( const std::string& name, const std::string& config )
{
    std::string filename{ "/etc/openvpn/" };
    filename.append( name );
    filename.append( ".conf" );

    std::ofstream f( filename );
    if ( !f.good() )
    {
        spdlog::error( "Could not write config file." );
        return false;
    }
    f << config;
    f.close();
    return true;
}

bool Ubus::UbusImpl::GetVPNConfigurations( std::vector<VPNConfiguration>& config )
{
    std::string directoryName{ "/etc/openvpn/" };

    try
    {
        for ( const auto& entry : std::experimental::filesystem::directory_iterator( directoryName ) )
        {
            auto path = entry.path();
            if ( path.extension() == ".conf" )
            {
                auto name   = path.stem();
                auto status = IsVpnRunning( name );
                config.push_back( VPNConfiguration{ .Name = name, .IsConnected = status } );
            }
        }
    }
    catch ( const std::exception& e )
    {
        spdlog::warn( "Exception iterating through configurations in /etc/openvpn: {]", e.what() );
        config.clear();
    }
    return true;
}

bool Ubus::UbusImpl::GetPortForwardings( std::vector<PortForwardingConfiguration>& config )
{
    struct uci_context* uci_ctx;
    struct uci_package* uci_firewall;
    struct uci_section* s;
    struct uci_element* e;

    uci_ctx = uci_alloc_context();
    uci_load( uci_ctx, "firewall", &uci_firewall );

    /* Check if redirect name is deleteable */
    uci_foreach_element( &uci_firewall->sections, e )
    {
        s = uci_to_section( e );

        if ( strcmp( s->type, "redirect" ) )
            continue;

        auto src = uci_lookup_option_string( uci_ctx, s, "src" );
        if ( !src || strcmp( src, "azurezone" ) )
        {
            continue;
        }

        auto name    = uci_lookup_option_string( uci_ctx, s, "name" );
        auto srcPort = uci_lookup_option_string( uci_ctx, s, "src_dport" );
        auto dstPort = uci_lookup_option_string( uci_ctx, s, "dest_port" );
        auto dstIP   = uci_lookup_option_string( uci_ctx, s, "dest_ip" );

        if ( !name || !srcPort || !dstPort || !dstIP )
        {
            spdlog::warn( "Found uci redirect config with missing fields." );
            continue;
        }

        PortForwardingConfiguration entry;
        entry.Name            = name;
        entry.SourcePort      = srcPort;
        entry.DestinationPort = dstPort;
        entry.DestinationIP   = dstIP;

        config.push_back( entry );
    }

    uci_free_context( uci_ctx );
    return true;
}

bool Ubus::UbusImpl::GetNetworkDetails( std::vector<NetworkDetail>& details )
{
    auto [ipAddrOutput, err1]    = runCmdWithOutput( "/sbin/ip --json addr show" );
    auto [ipv4RouteOutput, err2] = runCmdWithOutput( "/sbin/ip --json -4 route show" );
    auto [ipv6RouteOutput, err3] = runCmdWithOutput( "/sbin/ip --json -6 route show" );
    if ( err1 || err2 || err3 ) // one of the commands failed
    {
        return false;
    }

    rapidjson::Document addressesJson;
    addressesJson.Parse( ipAddrOutput );

    if ( addressesJson.HasParseError() )
    {
        spdlog::error( "Failed to parse ip addr show json output: {}", ipAddrOutput );
        return false;
    }
    if ( !addressesJson.IsArray() )
    {
        spdlog::error( "Expected array of interfaces, got {}", ipAddrOutput );
        return false;
    }

    rapidjson::Document routes4Json;
    routes4Json.Parse( ipv4RouteOutput );

    if ( routes4Json.HasParseError() )
    {
        spdlog::error( "Failed to parse ip -4 route json output: {}", ipv4RouteOutput );
        return false;
    }
    if ( !routes4Json.IsArray() )
    {
        spdlog::error( "Expected array of interfaces, got {}", ipv4RouteOutput );
        return false;
    }

    rapidjson::Document routes6Json;
    routes6Json.Parse( ipv6RouteOutput );

    if ( routes6Json.HasParseError() )
    {
        spdlog::error( "Failed to parse ip -6 route json output: {}", ipv6RouteOutput );
        return false;
    }
    if ( !routes6Json.IsArray() )
    {
        spdlog::error( "Expected array of interfaces, got {}", ipv6RouteOutput );
        return false;
    }

    for ( auto& interface : addressesJson.GetArray() )
    {
        NetworkDetail current;
        if ( !interface.IsObject() )
        {
            spdlog::warn( "Expected interface object in {}", ipAddrOutput );
            continue;
        }
        auto name = interface.FindMember( "ifname" );
        if ( name == interface.MemberEnd() || !name->value.IsString() )
        {
            spdlog::warn( "No ifname or wrong type in {}", ipAddrOutput );
            continue;
        }
        current.InterfaceName = name->value.GetString();

        auto mac = interface.FindMember( "address" );
        if ( mac == interface.MemberEnd() || !mac->value.IsString() )
        {
            spdlog::debug( "No MAC address or wrong type in {}", current.InterfaceName );
            // This is normal for some interfaces
        }
        else
        {
            current.MacAddress = mac->value.GetString();
        }

        auto flags = interface.FindMember( "flags" );
        if ( flags == interface.MemberEnd() || !flags->value.IsArray() )
        {
            spdlog::info( "No flags or wrong type in {}", ipAddrOutput );
        }
        else
        {
            for ( auto& flag : flags->value.GetArray() )
            {
                if ( !flag.IsString() )
                {
                    continue;
                }
                if ( !strcmp( flag.GetString(), "POINTOPOINT" ) )
                {
                    spdlog::info( "Found point to point interface {}", current.InterfaceName );
                    current.IsVpn = true;
                }
            }
        }

        auto addrInfo = interface.FindMember( "addr_info" );
        if ( addrInfo == interface.MemberEnd() || !addrInfo->value.IsArray() )
        {
            spdlog::warn( "No addr_info or wrong type in {}", ipAddrOutput );
            continue;
        }
        for ( auto& addressEntry : addrInfo->value.GetArray() )
        {
            if ( !addressEntry.IsObject() )
            {
                spdlog::warn( "Expected address object in {}", ipAddrOutput );
                continue;
            }
            auto scope = addressEntry.FindMember( "scope" );
            if ( scope == addressEntry.MemberEnd() || !scope->value.IsString() )
            {
                spdlog::warn( "No scope or wrong type in {}", ipAddrOutput );
                continue;
            }
            // Only show details for scope: global (not host or link)
            if ( strcmp( scope->value.GetString(), "global" ) )
            {
                continue;
            }
            auto family = addressEntry.FindMember( "family" );
            if ( family == addressEntry.MemberEnd() || !family->value.IsString() )
            {
                spdlog::warn( "No family or wrong type in {}", ipAddrOutput );
                continue;
            }
            auto local = addressEntry.FindMember( "local" );
            if ( local == addressEntry.MemberEnd() || !local->value.IsString() )
            {
                spdlog::warn( "No local address or wrong type in {}", ipAddrOutput );
                continue;
            }
            if ( !strcmp( family->value.GetString(), "inet" ) )
            {
                current.Ipv4Address = fmt::format( "{}", local->value.GetString() );
            }
            else if ( !strcmp( family->value.GetString(), "inet6" ) )
            {
                current.Ipv6Address = fmt::format( "{}", local->value.GetString() );
            }
            else
            {
                spdlog::debug( "Unknown family {} ignored.", family->value.GetString() );
            }
        }
        for ( auto& v4route : routes4Json.GetArray() )
        {
            if ( !v4route.IsObject() )
            {
                spdlog::error( "Expected route object in {}", ipv4RouteOutput );
                continue;
            }
            auto dev = v4route.FindMember( "dev" );
            if ( dev == v4route.MemberEnd() || !dev->value.IsString() )
            {
                spdlog::debug( "No dev or wrong type in {}", ipv4RouteOutput );
                continue;
            }
            if ( strcmp( dev->value.GetString(), current.InterfaceName.c_str() ) )
            {
                continue;
            }
            auto gateway = v4route.FindMember( "gateway" );
            if ( gateway == v4route.MemberEnd() || !gateway->value.IsString() )
            {
                spdlog::debug( "No gateway or wrong type in {}", ipv4RouteOutput );
                continue;
            }
            auto dst = v4route.FindMember( "dst" );
            if ( dst == v4route.MemberEnd() || !dst->value.IsString() )
            {
                spdlog::debug( "No dst or wrong type in {}", ipv4RouteOutput );
                continue;
            }
            if ( !strcmp( dst->value.GetString(), "default" ) )
            {
                current.Ipv4Gateway = gateway->value.GetString();
                break;
            }
        }

        for ( auto& v6route : routes6Json.GetArray() )
        {
            if ( !v6route.IsObject() )
            {
                spdlog::error( "Expected route object in {}", ipv6RouteOutput );
                continue;
            }
            auto dev = v6route.FindMember( "dev" );
            if ( dev == v6route.MemberEnd() || !dev->value.IsString() )
            {
                spdlog::debug( "No dev or wrong type in {}", ipv6RouteOutput );
                continue;
            }
            if ( strcmp( dev->value.GetString(), current.InterfaceName.c_str() ) )
            {
                continue;
            }
            auto gateway = v6route.FindMember( "gateway" );
            if ( gateway == v6route.MemberEnd() || !gateway->value.IsString() )
            {
                spdlog::debug( "No gateway or wrong type in {}", ipv6RouteOutput );
                continue;
            }
            auto dst = v6route.FindMember( "dst" );
            if ( dst == v6route.MemberEnd() || !dst->value.IsString() )
            {
                spdlog::debug( "No dst or wrong type in {}", ipv6RouteOutput );
                continue;
            }
            if ( !strcmp( dst->value.GetString(), "default" ) )
            {
                current.Ipv6Gateway = gateway->value.GetString();
                break;
            }
        }
        if ( !current.Ipv4Address.empty() || !current.Ipv6Address.empty() )
        {
            details.push_back( current );
        }
    }
    return true;
}

std::string Ubus::Name()
{
    return _impl->Name;
}

bool Ubus::SetName( const std::string& newName )
{
    _impl->Name = newName;
    return true;
}

std::string Ubus::Serial()
{
    return _impl->Serial;
}
std::string Ubus::Hostname()
{
    return _impl->Hostname;
}
std::string Ubus::Type()
{
    return _impl->Type;
}
std::string Ubus::FirmwareVersion()
{
    return _impl->FirmwareVersion;
}
std::string Ubus::MACAddress()
{
    return _impl->Mac;
}

bool Ubus::VPNConnectionActive()
{
    std::vector<Command::INetworkDetailSource::NetworkDetail> details;
    bool isVpnActive{ false };
    (void)_impl->GetNetworkDetails( details );
    for ( auto& entry : details )
    {
        isVpnActive |= entry.IsVpn;
    }
    return isVpnActive;
}

bool Ubus::DeleteRedirect( const std::string& name )
{
    return _impl->DeleteRedirect( name );
}

bool Ubus::ClearRedirect()
{
    return _impl->ClearRedirect();
}

bool Ubus::AddRedirect( const Command::IPortForwardingActor::PortForwardingConfiguration& config )
{
    return _impl->AddRedirect( config );
}

bool Ubus::ReloadFirewall()
{
    return _impl->ReloadFirewall();
}

Command::IVPNActor::ConnectionStatusVPN Ubus::StartVPN( const std::string& name, std::string& ipAddress )
{
    return _impl->StartVPN( name, ipAddress );
}

Command::IVPNActor::ConnectionStatusVPN Ubus::StopVPN( const std::string& name )
{
    return _impl->StopVPN( name );
}

bool Ubus::ConfigureVPN( const std::string& name, const std::string& config )
{
    return _impl->ConfigureVPN( name, config );
}

bool Ubus::GetVPNConfigurations( std::vector<VPNConfiguration>& config )
{
    return _impl->GetVPNConfigurations( config );
}

bool Ubus::GetPortForwardings( std::vector<PortForwardingConfiguration>& config )
{
    return _impl->GetPortForwardings( config );
}

bool Ubus::GetNetworkDetails( std::vector<NetworkDetail>& details )
{
    try
    {
        return _impl->GetNetworkDetails( details );
    }
    catch ( const std::exception& e )
    {
        spdlog::error( "Exception in GetNetworkDetails: {}", e.what() );
        ;
        return false;
    }
}
