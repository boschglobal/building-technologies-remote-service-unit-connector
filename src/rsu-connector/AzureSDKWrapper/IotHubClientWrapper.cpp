// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#include <cstring>
#include <vector>
#include <map>
#include <functional>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <mutex>
#include <condition_variable>

#include <iothub_device_client.h>
#include <iothub_device_client_ll.h>
#include <iothubtransportmqtt.h>
#include <iothub_client_options.h>
#include <azure_c_shared_utility/shared_util_options.h>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/bin_to_hex.h>

#include "IIotHubClient.h"
#include "IotHubClientWrapper.h"

using namespace std;

static inline const char* TextOrNull( const char* text )
{
    return text ? text : "(null)";
}

enum class MessageReaction
{
    Accepted,
    Rejected,
    Abandoned
};

static IOTHUBMESSAGE_DISPOSITION_RESULT_TAG ReactionToDisposition( MessageReaction response )
{
    auto retVal{ IOTHUBMESSAGE_ABANDONED };
    switch ( response )
    {
        case MessageReaction::Accepted:
            retVal = IOTHUBMESSAGE_ACCEPTED;
            break;
        case MessageReaction::Rejected:
            retVal = IOTHUBMESSAGE_REJECTED;
            break;
        case MessageReaction::Abandoned:
            retVal = IOTHUBMESSAGE_ABANDONED;
            break;
    }
    return retVal;
}

struct IotHubClientWrapper::IotHubClientWrapperImpl
{
    IotHubClientWrapperImpl( const std::string& iotHubUri, const std::string& deviceId, const std::string& statusFile);
    IotHubClientWrapperImpl( const std::string& connectionString, const std::string& statusFile);
    ~IotHubClientWrapperImpl();

    void SetLogTraceOption( bool value );
    void SendMessage( const MessageDataList& data,
                      const MessagePropertyList& properties,
                      std::shared_ptr<IMessageLifeTimeTracker> tracker );
    void SendMessage( const std::string& text,
                      const MessagePropertyList& properties,
                      std::shared_ptr<IMessageLifeTimeTracker> tracker );
    bool TriggerGetTwin();
    void SendReportedState( const std::string& reportedStateJson );
    void SetDeviceTwinHandler( std::function<void( const std::string&, bool )> handler );

    std::function<MessageReaction( const std::vector<uint8_t>& data )> CbByteMessage{ []( auto data ) {
        spdlog::info( "Binary message: {}", spdlog::to_hex( data ) );
        return MessageReaction::Abandoned;
    } };

    std::function<MessageReaction( const std::string& data )> CbTextMessage{ []( auto data ) {
        spdlog::info( "Text message: {}", data );
        return MessageReaction::Abandoned;
    } };

    std::string statusFileName;
    static void sStatusCallback( IOTHUB_CLIENT_CONNECTION_STATUS result,
                                 IOTHUB_CLIENT_CONNECTION_STATUS_REASON reason,
                                 void* userContextCallback )
    {
        if ( !userContextCallback )
        {
            spdlog::error( "Status callback with NULL user context." );
            return;
        }

        IotHubClientWrapperImpl* ctx = static_cast<IotHubClientWrapperImpl*>( userContextCallback );

        fstream status;
        status.open( ctx->statusFileName, ios::out );
        if ( !status )
        {
            spdlog::warn( "Status file cannot be created" );
        }
        else
        {
            switch ( reason )
            {
                case IOTHUB_CLIENT_CONNECTION_EXPIRED_SAS_TOKEN:
                    status << "{ \"status\": \"EXPIRED_SAS_TOKEN\" }\n";
                    break;
                case IOTHUB_CLIENT_CONNECTION_DEVICE_DISABLED:
                    status << "{ \"status\": \"DEVICE_DISABLED\" }\n";
                    break;
                case IOTHUB_CLIENT_CONNECTION_BAD_CREDENTIAL:
                    status << "{ \"status\": \"BAD_CREDENTIAL\" }\n";
                    break;
                case IOTHUB_CLIENT_CONNECTION_RETRY_EXPIRED:
                    status << "{ \"status\": \"RETRY_EXPIRED\" }\n";
                    break;
                case IOTHUB_CLIENT_CONNECTION_NO_NETWORK:
                    status << "{ \"status\": \"NO_NETWORK\" }\n";
                    break;
                case IOTHUB_CLIENT_CONNECTION_COMMUNICATION_ERROR:
                    status << "{ \"status\": \"COMMUNICATION_ERROR\" }\n";
                    break;
                case IOTHUB_CLIENT_CONNECTION_OK:
                    status << "{ \"status\": \"OK\" }\n";
                    break;
                case IOTHUB_CLIENT_CONNECTION_NO_PING_RESPONSE:
                    status << "{ \"status\": \"NO_PING_RESPONSE\" }\n";
                    break;
                default:
                    status << "{ \"status\": \"UNKNOWN\" }\n";
                    break;
            }
            status.close();
        }

        spdlog::info( "Received status {} reason {}", result, reason );
    }

    static IOTHUBMESSAGE_DISPOSITION_RESULT sMessageCallback( IOTHUB_MESSAGE_HANDLE message,
                                                              void* userContextCallback );

    static void sEventConfirmationCallback( IOTHUB_CLIENT_CONFIRMATION_RESULT result, void* userContextCallback );

    static void sDeviceTwinCallback( DEVICE_TWIN_UPDATE_STATE updateState,
                                     const unsigned char* payLoad,
                                     size_t size,
                                     void* userContextCallback );

    static int sDeviceMethodCallback( const char* method,
                                      const unsigned char* payload,
                                      size_t payloadSize,
                                      unsigned char** responsePtr,
                                      size_t* responseSizePtr,
                                      void* userContextCallback );
    static void sReportedStateCallback( int statusCode, void* userContextCallback );

    void SetupOptions();
    void SetupCallbacks();

    IOTHUB_DEVICE_CLIENT_HANDLE IotHubClientHandle{ NULL };

    // used by SendMessage()
    static std::mutex SendMessageLock;
    static std::vector<std::shared_ptr<IMessageLifeTimeTracker>> SendMessageTrackers;

    std::function<int( const std::string&, const std::string&, std::string& )> MethodHandler{
            []( auto& method, auto& argument, auto& response ) {
                spdlog::info( "Unhandled method call {} argument {}", method, argument );
                response = R"({"error": "unhandled method"})";
                return -1;
            } };

    std::function<void( const std::string& deviceTwin, bool partial )> DeviceTwinHandler{
            []( auto& deviceTwin, bool partial ) {
                spdlog::info( "Unhandled device Twin {} partial {}", deviceTwin, partial );
            } };
};

std::mutex IotHubClientWrapper::IotHubClientWrapperImpl::SendMessageLock;
std::vector<std::shared_ptr<IMessageLifeTimeTracker>> IotHubClientWrapper::IotHubClientWrapperImpl::SendMessageTrackers;

IotHubClientWrapper::IotHubClientWrapperImpl::IotHubClientWrapperImpl( const std::string& iotHubUri,
                                                                       const std::string& deviceId,
                                                                       const std::string& statusFile )
{
    if ( iotHubUri.empty() || deviceId.empty() )
    {
        throw std::invalid_argument( "Connection String" );
    }

    statusFileName = statusFile;

    IotHubClientHandle = IoTHubDeviceClient_CreateFromDeviceAuth( iotHubUri.c_str(), deviceId.c_str(), MQTT_Protocol );
    if ( !IotHubClientHandle )
    {
        throw std::runtime_error( "Create client from connection string failed." );
    }
    SetupOptions();
    SetupCallbacks();
    spdlog::debug( "IotHubClientWrapper" );
}

IotHubClientWrapper::IotHubClientWrapperImpl::IotHubClientWrapperImpl( const std::string& connectionString, const std::string& statusFile)
{
    if ( connectionString.empty() )
    {
        throw std::invalid_argument( "Connection String" );
    }

    statusFileName = statusFile;

    IotHubClientHandle = IoTHubDeviceClient_CreateFromConnectionString( connectionString.c_str(), MQTT_Protocol );
    if ( !IotHubClientHandle )
    {
        throw std::runtime_error( "Create client from connection string failed." );
    }
    SetupOptions();
    SetupCallbacks();
    spdlog::debug( "IotHubClientWrapper" );
}

IotHubClientWrapper::IotHubClientWrapperImpl::~IotHubClientWrapperImpl()
{
    if ( IotHubClientHandle )
    {
        IoTHubDeviceClient_Destroy( IotHubClientHandle );
        IotHubClientHandle = NULL;
        spdlog::debug( "~IotHubClientWrapper" );
    }
}

void IotHubClientWrapper::IotHubClientWrapperImpl::SetLogTraceOption( bool value )
{
    if ( IoTHubDeviceClient_SetOption( IotHubClientHandle, "logtrace", &value ) != IOTHUB_CLIENT_OK )
    {
        throw std::runtime_error( "Set logtrace option." );
    }
}

void IotHubClientWrapper::IotHubClientWrapperImpl::SendMessage( const MessageDataList& data,
                                                                const MessagePropertyList& properties,
                                                                std::shared_ptr<IMessageLifeTimeTracker> tracker )
{
    auto message = IoTHubMessage_CreateFromByteArray( &data[0], data.size() );
    if ( !message )
    {
        spdlog::error( "Could not create message from byte array." );
    }

    for ( auto keyValuePair : properties )
    {
        IoTHubMessage_SetProperty( message, keyValuePair.first.c_str(), keyValuePair.second.c_str() );
    }

    if ( tracker )
    {
        std::unique_lock<std::mutex> lock( SendMessageLock );
        tracker->SetReference( message );
        tracker->SetState( IMessageLifeTimeTracker::DeliveryState::Sending );
        SendMessageTrackers.push_back( tracker ); // need to keep shared_ptr alive until callback has been processed
    }

    auto success = IoTHubDeviceClient_SendEventAsync( IotHubClientHandle,
                                                      message,
                                                      sEventConfirmationCallback,
                                                      static_cast<void*>( message ) );
    if ( success != IOTHUB_CLIENT_OK )
    {
        spdlog::error( "Sending byte array message failed {}", success );
        throw std::runtime_error( "Message send failed." );
    }
}

void IotHubClientWrapper::IotHubClientWrapperImpl::SendMessage( const std::string& text,
                                                                const MessagePropertyList& properties,
                                                                std::shared_ptr<IMessageLifeTimeTracker> tracker )
{
    auto message = IoTHubMessage_CreateFromString( text.c_str() );
    if ( !message )
    {
        spdlog::error( "Could not create message from string." );
    }

    for ( auto keyValuePair : properties )
    {
        IoTHubMessage_SetProperty( message, keyValuePair.first.c_str(), keyValuePair.second.c_str() );
    }

    if ( tracker )
    {
        std::unique_lock<std::mutex> lock( SendMessageLock );
        tracker->SetReference( message );
        tracker->SetState( IMessageLifeTimeTracker::DeliveryState::Sending );
        SendMessageTrackers.push_back( tracker ); // need to keep shared_ptr alive until callback has been processed
    }

    auto success = IoTHubDeviceClient_SendEventAsync( IotHubClientHandle,
                                                      message,
                                                      sEventConfirmationCallback,
                                                      static_cast<void*>( message ) );
    if ( success != IOTHUB_CLIENT_OK )
    {
        spdlog::error( "Sending byte array message failed {}", success );
        throw std::runtime_error( "Message send failed." );
    }
}

bool IotHubClientWrapper::IotHubClientWrapperImpl::TriggerGetTwin()
{
    auto success = IoTHubDeviceClient_GetTwinAsync( IotHubClientHandle, sDeviceTwinCallback, this );
    if ( IOTHUB_CLIENT_OK != success )
    {
        spdlog::error( "Device twin get failed {}", success );
        return false;
    }
    return true;
}

void IotHubClientWrapper::IotHubClientWrapperImpl::SendReportedState( const std::string& reportedStateJson )
{
    auto success =
            IoTHubDeviceClient_SendReportedState( IotHubClientHandle,
                                                  reinterpret_cast<const unsigned char*>( reportedStateJson.c_str() ),
                                                  reportedStateJson.size(),
                                                  sReportedStateCallback,
                                                  this );
    if ( IOTHUB_CLIENT_OK != success )
    {
        spdlog::error( "Send reported device state failed {}", success );
        throw std::runtime_error( "Could not send reported device state." );
    }
}

void IotHubClientWrapper::IotHubClientWrapperImpl::SetDeviceTwinHandler(
        std::function<void( const std::string&, bool )> handler )
{
    DeviceTwinHandler = handler;
    // This requests a transfer of the device twin state on each connect, which costs at least one message.
    auto success = IoTHubDeviceClient_SetDeviceTwinCallback( IotHubClientHandle, sDeviceTwinCallback, this );
    if ( IOTHUB_CLIENT_OK != success )
    {
        spdlog::error( "Set device twin callback failed {}", success );
        throw std::runtime_error( "Set device twin callback." );
    }
}

void IotHubClientWrapper::IotHubClientWrapperImpl::SetupOptions()
{
    static const char modelId[] = "dtmi:com:bosch:bt:RemoteServicesFuture:RemoteServiceUnit;1";

    auto retValSetModel = IoTHubDeviceClient_SetOption( IotHubClientHandle, OPTION_MODEL_ID, modelId );
    if ( IOTHUB_CLIENT_OK != retValSetModel )
    {
        spdlog::warn( "Set model id failed {}", retValSetModel );
        // this error is not fatal
    }

    // source: https://www.openssl.org/docs/man1.0.2/man1/ciphers.html for TLS1.2 excluding CBC ciphers
    const char* ciphers = "ECDH-ECDSA-AES128-GCM-SHA256:"
                          "ECDH-ECDSA-AES256-GCM-SHA384:"
                          "ECDHE-RSA-AES128-GCM-SHA256:"
                          "ECDHE-RSA-AES256-GCM-SHA384:"
                          "ECDHE-ECDSA-AES128-GCM-SHA256:"
                          "ECDHE-ECDSA-AES256-GCM-SHA384:"
                          "AES128-GCM-SHA256:"
                          "AES256-GCM-SHA384:"
                          "DH-RSA-AES128-GCM-SHA256:"
                          "DH-RSA-AES256-GCM-SHA384:"
                          "DHE-RSA-AES128-GCM-SHA256:"
                          "DHE-RSA-AES256-GCM-SHA384:"
                          "ECDH-RSA-AES128-GCM-SHA256:"
                          "ECDH-RSA-AES256-GCM-SHA384";
    auto retValSsl = IoTHubDeviceClient_SetOption( IotHubClientHandle, OPTION_OPENSSL_CIPHER_SUITE, ciphers );
    if ( IOTHUB_CLIENT_OK != retValSsl )
    {
        spdlog::warn( "Set SSL ciphers failed {}", retValSsl );
        // this error is not fatal.
    }
}

void IotHubClientWrapper::IotHubClientWrapperImpl::SetupCallbacks()
{
    auto success = IoTHubDeviceClient_SetConnectionStatusCallback( IotHubClientHandle, &sStatusCallback, this );
    if ( IOTHUB_CLIENT_OK != success )
    {
        IoTHubDeviceClient_Destroy( IotHubClientHandle );
        IotHubClientHandle = NULL;
        spdlog::error( "Set connection status callback failed {}", success );
        throw std::runtime_error( "Set connection status callback." );
    }
    success = IoTHubDeviceClient_SetMessageCallback( IotHubClientHandle, &sMessageCallback, this );
    if ( IOTHUB_CLIENT_OK != success )
    {
        IoTHubDeviceClient_Destroy( IotHubClientHandle );
        IotHubClientHandle = NULL;
        spdlog::error( "Set message callback failed {}", success );
        throw std::runtime_error( "Set message callback." );
    }
    success = IoTHubDeviceClient_SetDeviceMethodCallback( IotHubClientHandle, &sDeviceMethodCallback, this );
    if ( IOTHUB_CLIENT_OK != success )
    {
        IoTHubDeviceClient_Destroy( IotHubClientHandle );
        IotHubClientHandle = NULL;
        spdlog::error( "Set method callback failed {}", success );
        throw std::runtime_error( "Set method callback." );
    }
}

IOTHUBMESSAGE_DISPOSITION_RESULT IotHubClientWrapper::IotHubClientWrapperImpl::sMessageCallback(
        IOTHUB_MESSAGE_HANDLE message,
        void* userContextCallback )
{
    auto retVal{ IOTHUBMESSAGE_ABANDONED };

    if ( !userContextCallback )
    {
        spdlog::error( "Message callback with NULL user context." );
        return retVal;
    }
    if ( !message )
    {
        spdlog::error( "Message callback with NULL message." );
        return retVal;
    }

    IotHubClientWrapperImpl* ctx = static_cast<IotHubClientWrapperImpl*>( userContextCallback );
    auto messageId               = IoTHubMessage_GetMessageId( message );
    auto correlationId           = IoTHubMessage_GetCorrelationId( message );
    auto messageType             = IoTHubMessage_GetContentType( message );
    auto contentType             = IoTHubMessage_GetContentTypeSystemProperty( message );
    auto encoding                = IoTHubMessage_GetContentEncodingSystemProperty( message );
    spdlog::info( "Received message id {} correlation id {} with message type {} content type {} encoding ",
                  TextOrNull( messageId ),
                  TextOrNull( correlationId ),
                  messageType,
                  TextOrNull( contentType ),
                  TextOrNull( encoding ) );

    switch ( messageType )
    {
        case IOTHUBMESSAGE_STRING: {
            auto data = IoTHubMessage_GetString( message );
            if ( !data )
            {
                spdlog::error( "Getting string from message failed." );
            }
            else
            {
                std::string text{ data };
                retVal = ReactionToDisposition( ctx->CbTextMessage( text ) );
            }
            break;
        }

        case IOTHUBMESSAGE_BYTEARRAY: {
            const uint8_t* bufferPtr;
            size_t size;
            auto success = IoTHubMessage_GetByteArray( message, &bufferPtr, &size );
            if ( success != IOTHUB_MESSAGE_OK )
            {
                spdlog::error( "Getting byte array from message failed." );
            }
            else if ( bufferPtr && size )
            {
                std::vector<uint8_t> data{ bufferPtr, bufferPtr + size };
                retVal = ReactionToDisposition( ctx->CbByteMessage( data ) );
            }
            else
            {
                spdlog::error( "Byte array message with empty data." );
            }
            break;
        }

        default:
            spdlog::error( "Invalid message type received from IOTHUB." );
    }
    return retVal;
}

void IotHubClientWrapper::IotHubClientWrapperImpl::sEventConfirmationCallback( IOTHUB_CLIENT_CONFIRMATION_RESULT result,
                                                                               void* userContextCallback )
{
    if ( !userContextCallback )
    {
        spdlog::error( "Event confirmation callback with NULL user context." );
        return;
    }
    std::unique_lock<std::mutex> lock( SendMessageLock );
    auto it = std::find_if( SendMessageTrackers.begin(),
                            SendMessageTrackers.end(),
                            [userContextCallback]( auto& item ) { return item->Reference() == userContextCallback; } );
    if ( it != SendMessageTrackers.end() )
    {
        spdlog::info( "IotHubClient: reporting result {} to tracker", result );
        auto item = *it;
        switch ( result )
        {
            case IOTHUB_CLIENT_CONFIRMATION_OK:
                item->SetState( IMessageLifeTimeTracker::DeliveryState::SentSuccessful );
                break;
            case IOTHUB_CLIENT_CONFIRMATION_BECAUSE_DESTROY:
                item->SetState( IMessageLifeTimeTracker::DeliveryState::ErrorDestroyed );
                break;
            case IOTHUB_CLIENT_CONFIRMATION_MESSAGE_TIMEOUT:
                item->SetState( IMessageLifeTimeTracker::DeliveryState::ErrorTimeout );
                break;
            default:
                item->SetState( IMessageLifeTimeTracker::DeliveryState::UnspecifiedError );
                break;
        }
        item->SetReference( nullptr );
        SendMessageTrackers.erase( it ); // release the smart_ptr
    }
    IoTHubMessage_Destroy( static_cast<IOTHUB_MESSAGE_HANDLE>( userContextCallback ) );
}

void IotHubClientWrapper::IotHubClientWrapperImpl::sDeviceTwinCallback( DEVICE_TWIN_UPDATE_STATE updateState,
                                                                        const unsigned char* payLoad,
                                                                        size_t size,
                                                                        void* userContextCallback )
{
    if ( !payLoad )
    {
        spdlog::error( "Device twin callback state {} with no payload", updateState );
        return;
    }
    std::string deviceTwinData( payLoad, payLoad + size );
    spdlog::info( "Device twin callback state {} hubUri {}", updateState, deviceTwinData );
    if ( !userContextCallback )
    {
        spdlog::error( "Device twin callback with NULL user context." );
        return;
    }
    IotHubClientWrapperImpl* ctx = static_cast<IotHubClientWrapperImpl*>( userContextCallback );
    ctx->DeviceTwinHandler( deviceTwinData, ( updateState == DEVICE_TWIN_UPDATE_PARTIAL ) );
}

int IotHubClientWrapper::IotHubClientWrapperImpl::sDeviceMethodCallback( const char* method,
                                                                         const unsigned char* payload,
                                                                         size_t payloadSize,
                                                                         unsigned char** responsePtr,
                                                                         size_t* responseSizePtr,
                                                                         void* userContextCallback )
{
    std::string input( payload, payload + payloadSize );

    spdlog::info( "Device method callback name {} payload {}", method, input );
    if ( !userContextCallback )
    {
        spdlog::error( "Device registration callback with NULL user context." );
        return -1;
    }
    IotHubClientWrapperImpl* ctx = static_cast<IotHubClientWrapperImpl*>( userContextCallback );
    std::string responseString;
    auto status                   = ctx->MethodHandler( method, input, responseString );
    const auto responseSize       = responseString.size();
    unsigned char* responseMemory = static_cast<unsigned char*>( malloc( responseSize ) );
    if ( responseMemory )
    {
        memcpy( responseMemory, responseString.c_str(), responseSize );
        *responseSizePtr = responseSize;
        *responsePtr     = responseMemory; //  TODO: check whether this allocation causes a memory leak
    }
    else
    {
        *responseSizePtr = 0;
    }
    return status;
}

void IotHubClientWrapper::IotHubClientWrapperImpl::sReportedStateCallback( int statusCode, void* userContextCallback )
{
    spdlog::info( "Send reported state response {}", statusCode );
}

IotHubClientWrapper::IotHubClientWrapper( const std::string& iotHubUri, const std::string& deviceId, const std::string& statusFile )
    : _impl( std::make_shared<IotHubClientWrapperImpl>( iotHubUri, deviceId, statusFile) )
{
    spdlog::debug( "Created IotHub client." );
}

IotHubClientWrapper::IotHubClientWrapper( const std::string& connectionString, const std::string& statusFile )
    : _impl( std::make_shared<IotHubClientWrapperImpl>( connectionString, statusFile ) )
{
    spdlog::debug( "Created IotHub client." );
}

void IotHubClientWrapper::SetLogTraceOption( bool value )
{
    _impl->SetLogTraceOption( value );
}

void IotHubClientWrapper::SendMessage( const MessageDataList& data,
                                       const MessagePropertyList& properties,
                                       std::shared_ptr<IMessageLifeTimeTracker> tracker )
{
    _impl->SendMessage( data, properties, tracker );
}

void IotHubClientWrapper::SendMessage( const std::string& text,
                                       const MessagePropertyList& properties,
                                       std::shared_ptr<IMessageLifeTimeTracker> tracker )
{
    _impl->SendMessage( text, properties, tracker );
}

bool IotHubClientWrapper::TriggerGetTwin()
{
    return _impl->TriggerGetTwin();
}

void IotHubClientWrapper::SendReportedState( const std::string& reportedStateJson )
{
    _impl->SendReportedState( reportedStateJson );
}

void IotHubClientWrapper::SetMethodHandler(
        std::function<int( const std::string&, const std::string&, std::string& )> handler )
{
    _impl->MethodHandler = handler;
}

void IotHubClientWrapper::SetDeviceTwinHandler( std::function<void( const std::string&, bool )> handler )
{
    _impl->SetDeviceTwinHandler( handler );
}
