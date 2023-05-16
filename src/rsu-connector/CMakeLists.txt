cmake_minimum_required(VERSION 3.14)

# The executables with code coverage generate *.gcda files when they are executed.
# If these files persist after a recompilation, the next call to the executable will create a large amount of warnings.
# This target removes these files on rebuild. All targets that require code coverage should depend on it.
add_custom_target(clean_coverage
  COMMAND find ${CMAKE_BINARY_DIR} -name \*.gcda -delete
)

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
FIND_LIBRARY(cppbase64 NAMES cppbase64)
FIND_PATH(iothub_client_include_dir azureiot/iothub.h)

set(azurelibs
    ${prov_device_client}
    ${prov_device_ll_client}
    ${iothub_client}
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

FIND_LIBRARY(ubox NAMES ubox)
FIND_LIBRARY(ubus NAMES ubus)
FIND_LIBRARY(uci NAMES uci)
FIND_LIBRARY(spdlog NAMES spdlog)
FIND_LIBRARY(fmt NAMES fmt)

SET(EXECUTABLE_COMPILE_OPTIONS
    -Os -ggdb -fPIE
    -Wall -Wextra -Wno-unused-parameter -Wformat=2 -Wformat-security -Wstrict-overflow
    -fstack-protector-all -fno-omit-frame-pointer -fno-common
    -DSPDLOG_FMT_EXTERNAL)
# SHELL: prefix turns off deduplication for these entries, which would remove but one of the multiple -z options
SET(EXECUTABLE_LINK_OPTIONS -ggdb -fno-omit-frame-pointer -pie "SHELL:-z noexecstack" "SHELL:-z relro " "SHELL:-z now " "SHELL:-z nodlopen")

SET(TEST_COMPILE_OPTIONS 
    -O0 -ggdb -fPIE
    -Wall -Wextra  -Wno-unused-parameter -Wformat=2 -Wformat-security -Wstrict-overflow
    -fstack-protector-all -fno-omit-frame-pointer -fno-common
    -DUNITTEST_PUBLIC_PIMPL -DDEBUG -DSPDLOG_FMT_EXTERNAL
    -fprofile-arcs -ftest-coverage --coverage )

SET(TEST_LINK_OPTIONS --coverage ${EXECUTABLE_LINK_OPTIONS})

set(TARGET_NAME TelemetryForwarder)

set(SRC 
    AzureSDKWrapper/IotHubClientWrapper.cpp 
    AzureSDKWrapper/ProvisioningClientWrapper.cpp
    AzureSDKWrapper/IotHubFactory.cpp
    AzureSDKWrapper/CustomHSM.cpp
    Configuration/Configuration.cpp
    HttpHandler/HttpHandlerFactory.cpp
    HttpHandler/HttpClient.cpp
    HttpHandler/HttpServer.cpp
    main/IotHubOrNull.cpp
    main/telemetry-main.cpp 
)

set(INC 
    AzureSDKWrapper/IotHubClientWrapper.h 
    AzureSDKWrapper/ProvisioningClientWrapper.h
    AzureSDKWrapper/IIotHubClient.h 
    AzureSDKWrapper/IProvisioningClient.h
    AzureSDKWrapper/IotHubFactory.h
    AzureSDKWrapper/CustomHSM.h
    Configuration/Configuration.h
    HttpHandler/HttpHandlerFactory.h
    HttpHandler/IHttpClient.h
    HttpHandler/IHttpServer.h
    HttpHandler/HttpClient.h
    HttpHandler/HttpServer.h
    main/IotHubOrNull.h
)

add_executable(${TARGET_NAME} ${SRC} ${INC} ${BASE64SRC})
target_link_libraries(${TARGET_NAME} PRIVATE ${azurelibs} uhttp ${spdlog} ${fmt} ssl crypto oatpp::oatpp cppbase64 )
target_include_directories(${TARGET_NAME} PRIVATE . ${oatpp_INCLUDE_DIRS} ${iothub_client_include_dir}/azureiot)
target_compile_options(${TARGET_NAME} PRIVATE ${EXECUTABLE_COMPILE_OPTIONS})
set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 17)
# SHELL: prefix turns off deduplication for these entries, which would remove but one of the multiple -z options
target_link_options(${TARGET_NAME} BEFORE PRIVATE ${EXECUTABLE_LINK_OPTIONS})

if( NOT ( ${ubox} MATCHES "NOTFOUND" OR ${ubus} MATCHES "NOTFOUND" OR ${uci} MATCHES "NOTFOUND" ) )

set(TARGET_NAME rsu-connector)
project($TARGET_NAME)

set(SRC 
    main/main.cpp
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

set(INC 
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

add_executable(${TARGET_NAME} ${SRC} ${INC} ${BASE64SRC} )
target_link_libraries(${TARGET_NAME} PRIVATE ${azurelibs} curl uhttp oatpp::oatpp ${ubus} ${ubox} ${uci} ${spdlog} ${fmt} stdc++fs ssl crypto spdlog cppbase64)
target_include_directories(${TARGET_NAME} PRIVATE . ${oatpp_INCLUDE_DIRS} ${iothub_client_include_dir}/azureiot)
target_compile_options(${TARGET_NAME} PRIVATE ${EXECUTABLE_COMPILE_OPTIONS})
set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 17)
target_link_options(${TARGET_NAME} BEFORE PRIVATE ${EXECUTABLE_LINK_OPTIONS})

endif()

set(TESTSRC
    unittest/ConfigurationTest.cpp
    unittest/IotHubOrNullTest.cpp
    unittest/MessageCollectorTest.cpp
    unittest/ReliableMessageDispatcherTest.cpp
    unittest/JobSchedulerTest.cpp
    unittest/HttpHandlerTest.cpp
    unittest/DeviceTwinTest.cpp
    unittest/CustomHSMTest.cpp
    unittest/CommandAddPortForwardingTest.cpp
    unittest/CommandRemovePortForwardingTest.cpp
    unittest/CommandClearPortForwardingTest.cpp
    unittest/CommandGetPortForwardingsTest.cpp
    unittest/CommandStartVPNTest.cpp
    unittest/CommandStopVPNTest.cpp
    unittest/CommandConfigureVPNConnectionTest.cpp
    unittest/CommandGetVPNConfigurationsTest.cpp
    unittest/CommandGetNetworkDetailsTest.cpp
    unittest/CommandRestartConnectorTest.cpp
    unittest/UbusTest.cpp
    unittest/testmain.cpp
    )

set(TESTINC
    unittest/MockIotClient.h
    unittest/MockPortForwardingActor.h
    unittest/MockVPNActor.h
)

set(DOCTESTINC ../external/doctest)

set(TARGET_NAME UT_rsu-connector)
add_executable(${TARGET_NAME} ${TESTSRC} ${TESTINC} ${INC} ${BASE64SRC})
add_dependencies(${TARGET_NAME} clean_coverage)
target_include_directories(${TARGET_NAME} PRIVATE . ${DOCTESTINC} ${oatpp_INCLUDE_DIRS} ${iothub_client_include_dir}/azureiot)
target_link_libraries(${TARGET_NAME} PRIVATE oatpp::oatpp ${ubox} ${uci} ${spdlog} ${fmt} stdc++fs ssl crypto cppbase64)
target_compile_options(${TARGET_NAME} PRIVATE ${TEST_COMPILE_OPTIONS})
set_property(TARGET ${TARGET_NAME} PROPERTY CXX_STANDARD 17)
target_link_options(${TARGET_NAME} BEFORE PRIVATE ${TEST_LINK_OPTIONS})

