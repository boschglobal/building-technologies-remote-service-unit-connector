cmake_minimum_required(VERSION 3.15)
PROJECT(rsu-connector CXX)
SET(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
SET(CMAKE_INCLUDE_CURRENT_DIR ON)

FIND_PACKAGE(oatpp 1.2.5 EXACT REQUIRED)

FIND_LIBRARY(iothub_client NAMES iothub_client PATH_SUFFIXES azure-iothub-client-hsm-type-custom)
FIND_LIBRARY(prov_auth_client NAMES prov_auth_client PATH_SUFFIXES azure-iothub-client-hsm-type-custom)
FIND_LIBRARY(prov_device_client NAMES prov_device_client PATH_SUFFIXES azure-iothub-client-hsm-type-custom)
FIND_LIBRARY(prov_device_ll_client NAMES prov_device_ll_client PATH_SUFFIXES azure-iothub-client-hsm-type-custom)
FIND_LIBRARY(iothub_client_amqp_transport NAMES iothub_client_amqp_transport PATH_SUFFIXES azure-iothub-client-hsm-type-custom)
FIND_LIBRARY(iothub_client_amqp_ws_transport NAMES iothub_client_amqp_ws_transport PATH_SUFFIXES azure-iothub-client-hsm-type-custom)
FIND_LIBRARY(iothub_client_http_transport NAMES iothub_client_http_transport PATH_SUFFIXES azure-iothub-client-hsm-type-custom)
FIND_LIBRARY(iothub_client_mqtt_transport NAMES iothub_client_mqtt_transport PATH_SUFFIXES azure-iothub-client-hsm-type-custom)
FIND_LIBRARY(iothub_client_mqtt_ws_transport NAMES iothub_client_mqtt_ws_transport PATH_SUFFIXES azure-iothub-client-hsm-type-custom)
FIND_LIBRARY(prov_mqtt_transport NAMES prov_mqtt_transport PATH_SUFFIXES azure-iothub-client-hsm-type-custom)
FIND_LIBRARY(prov_amqp_transport NAMES prov_amqp_transport PATH_SUFFIXES azure-iothub-client-hsm-type-custom)
FIND_LIBRARY(prov_http_transport NAMES prov_http_transport PATH_SUFFIXES azure-iothub-client-hsm-type-custom)
FIND_LIBRARY(hsm_security_client NAMES hsm_security_client PATH_SUFFIXES azure-iothub-client-hsm-type-custom)
FIND_LIBRARY(serializer NAMES serializer PATH_SUFFIXES azure-iothub-client-hsm-type-custom)
FIND_LIBRARY(parson NAMES parson)
FIND_LIBRARY(umqtt NAMES umqtt)
FIND_LIBRARY(aziotsharedutil NAMES aziotsharedutil_dll)
FIND_LIBRARY(ubox NAMES ubox)
FIND_LIBRARY(ubus NAMES ubus)
FIND_LIBRARY(uci NAMES uci)
FIND_LIBRARY(spdlog NAMES spdlog)
FIND_LIBRARY(fmt NAMES fmt)
FIND_LIBRARY(uhttp NAMES uhttp)
FIND_LIBRARY(cppbase64 NAMES cppbase64)

SET(azurelibs
	${iothub_client}
	${prov_device_client}
	${prov_device_ll_client}
	${aziotsharedutil}
	${prov_http_transport}
	${prov_mqtt_transport}
	${prov_amqp_transport}
	${iothub_client_amqp_transport}
	${iothub_client_amqp_ws_transport}
	${iothub_client_http_transport}
	${iothub_client_mqtt_transport}
	${iothub_client_mqtt_ws_transport}
	${prov_auth_client}
	${hsm_security_client}
	${serializer}
	${parson}
	${umqtt}
)

FIND_PATH(iothub_client_include_dir azureiot/iothub.h)

# BUILD librsu
SET(TARGET_NAME rsu)

SET(RSU_CON_LIB_SRC
	AzureSDKWrapper/IotHubClientWrapper.cpp
	AzureSDKWrapper/ProvisioningClientWrapper.cpp
	AzureSDKWrapper/IotHubFactory.cpp
	AzureSDKWrapper/MessageCollector.cpp
	AzureSDKWrapper/MethodRouter.cpp
	AzureSDKWrapper/ReliableMessageDispatcher.cpp
	AzureSDKWrapper/CustomHSM.cpp
	Configuration/Configuration.cpp
	Commands/AddPortForwarding.cpp
	Commands/GetNetworkDetails.cpp
	Commands/RemovePortForwarding.cpp
	Commands/ClearPortForwardings.cpp
	Commands/GetPortForwardings.cpp
	Commands/StartVPNConnection.cpp
	Commands/StopVPNConnection.cpp
	Commands/GetVPNConfigurations.cpp
	Commands/ConfigureVPNConnection.cpp
	Commands/RestartConnector.cpp
	HttpHandler/HttpHandlerFactory.cpp
	HttpHandler/HttpClient.cpp
	HttpHandler/HttpServer.cpp
	JobScheduler/JobScheduler.cpp
	DeviceTwin/DeviceTwin.cpp
	main/IotHubOrNull.cpp
	UBus/UBus.cpp
)

SET(RSU_CON_LIB_INC
	AzureSDKWrapper/IotHubClientWrapper.h
	AzureSDKWrapper/ProvisioningClientWrapper.h
	AzureSDKWrapper/IIotHubClient.h
	AzureSDKWrapper/IProvisioningClient.h
	AzureSDKWrapper/IotHubFactory.h
	AzureSDKWrapper/MessageCollector.h
	AzureSDKWrapper/MethodRouter.h
	AzureSDKWrapper/ReliableMessageDispatcher.h
	AzureSDKWrapper/CustomHSM.h
	Configuration/Configuration.h
	Commands/ICommand.h
	Commands/AddPortForwarding.h
	Commands/GetNetworkDetails.h
	Commands/RemovePortForwarding.h
	Commands/ClearPortForwardings.h
	Commands/GetPortForwardings.h
	Commands/IPortForwardingActor.h
	Commands/IVPNActor.h
	Commands/StartVPNConnection.h
	Commands/StopVPNConnection.h
	Commands/GetVPNConfigurations.h
	Commands/ConfigureVPNConnection.h
	Commands/RestartConnector.h
	JobScheduler/JobScheduler.h
	JobScheduler/IJobScheduler.h
	DeviceTwin/DeviceTwin.h
	DeviceTwin/IDeviceInformation.h
	HttpHandler/HttpHandlerFactory.h
	HttpHandler/IHttpClient.h
	HttpHandler/IHttpServer.h
	HttpHandler/HttpClient.h
	HttpHandler/HttpServer.h
	main/IotHubOrNull.h
	UBus/UBus.h
)

ADD_LIBRARY(${TARGET_NAME} STATIC ${RSU_CON_LIB_SRC} ${RSU_CON_LIB_INC})
SET_PROPERTY(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 17)

TARGET_INCLUDE_DIRECTORIES(${TARGET_NAME} PRIVATE ${oatpp_INCLUDE_DIRS} ${iothub_client_include_dir}/azureiot)

TARGET_LINK_LIBRARIES(${TARGET_NAME} ${azurelibs})
TARGET_LINK_LIBRARIES(${TARGET_NAME} curl)
TARGET_LINK_LIBRARIES(${TARGET_NAME} ${uhttp})
TARGET_LINK_LIBRARIES(${TARGET_NAME} oatpp::oatpp)
TARGET_LINK_LIBRARIES(${TARGET_NAME} ${ubus})
TARGET_LINK_LIBRARIES(${TARGET_NAME} ${ubox})
TARGET_LINK_LIBRARIES(${TARGET_NAME} ${uci})
TARGET_LINK_LIBRARIES(${TARGET_NAME} ${spdlog})
TARGET_LINK_LIBRARIES(${TARGET_NAME} ${fmt})
TARGET_LINK_LIBRARIES(${TARGET_NAME} stdc++fs)
TARGET_LINK_LIBRARIES(${TARGET_NAME} ssl)
TARGET_LINK_LIBRARIES(${TARGET_NAME} crypto)
TARGET_LINK_LIBRARIES(${TARGET_NAME} cppbase64)

INSTALL(TARGETS ${TARGET_NAME} DESTINATION lib)

# BUILD rsu-connector
SET(TARGET_NAME rsu-connector)

SET(RSU_CON_SRC
	main/main.cpp
)

ADD_EXECUTABLE(${TARGET_NAME} ${RSU_CON_SRC})
SET_PROPERTY(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 17)
TARGET_LINK_LIBRARIES(${TARGET_NAME} PRIVATE ${spdlog} rsu)

INSTALL(TARGETS ${TARGET_NAME} DESTINATION bin)
