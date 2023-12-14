// Copyright (c) 2022 - for information on the respective copyright owner see the NOTICE file and/or the repository
// https://github.com/boschglobal/building-technologies-remote-service-unit-connector.
//
// SPDX-License-Identifier: Apache-2.0
//--- END HEADER ---

#include "AzureSDKWrapper/CustomHSM.cpp"
#include <doctest/doctest.h>
#include <spdlog/spdlog.h>

// This is a bit pointless, since the constructor and destructor are currently default and the class does not contain attributes
TEST_CASE( "CustomHsm - can be created" )
{
    auto hsm = std::make_shared<CustomHsm>();
}

TEST_CASE( "CustomHsm - SAS can be set through class interface and read by C" )
{
    auto hsm                  = std::make_shared<CustomHsm>();
    std::string expectedRegId = "testRegId";
    std::string expectedSas   = "testSas";
    hsm->setRegistrationId( expectedRegId );
    hsm->setSas( expectedSas );

    auto cInterface = hsm_client_key_interface();

    REQUIRE( cInterface != NULL );

    auto instance = cInterface->hsm_client_key_create(); // this does nothing, but lets observe the interface semantics

    auto regId = cInterface->hsm_client_get_registration_name( instance );
    auto sas   = cInterface->hsm_client_get_symm_key( instance );

    CHECK( expectedRegId == regId );
    CHECK( expectedSas == sas );
    free( regId );
    free( sas );

    cInterface->hsm_client_key_destroy( instance ); // this does nothing, but lets observe the interface semantics
}

TEST_CASE( "CustomHsm - SAS with null pointers through C interface" )
{
    auto hsm                  = std::make_shared<CustomHsm>();
    std::string expectedRegId = "testRegIdXX";
    std::string expectedSas   = "testSasXX";

    auto cInterface = hsm_client_key_interface();

    REQUIRE( cInterface != NULL );

    auto instance = cInterface->hsm_client_key_create(); // this does nothing, but lets observe the interface semantics

    CHECK( 1 == cInterface->hsm_client_set_symm_key_info( instance, expectedRegId.c_str(), NULL ) );
    CHECK( 1 == cInterface->hsm_client_set_symm_key_info( instance, NULL, expectedSas.c_str() ) );

    cInterface->hsm_client_key_destroy( instance ); // this does nothing, but lets observe the interface semantics
}

TEST_CASE( "CustomHsm - SAS can be set and read through C interface" )
{
    auto hsm                  = std::make_shared<CustomHsm>();
    std::string expectedRegId = "testRegIdXX";
    std::string expectedSas   = "testSasXX";

    auto cInterface = hsm_client_key_interface();

    REQUIRE( cInterface != NULL );

    auto instance = cInterface->hsm_client_key_create(); // this does nothing, but lets observe the interface semantics

    CHECK( 0 == cInterface->hsm_client_set_symm_key_info( instance, expectedRegId.c_str(), expectedSas.c_str() ) );

    auto regId = cInterface->hsm_client_get_registration_name( instance );
    auto sas   = cInterface->hsm_client_get_symm_key( instance );

    CHECK( expectedRegId == regId );
    CHECK( expectedSas == sas );

    free( regId );
    free( sas );

    cInterface->hsm_client_key_destroy( instance ); // this does nothing, but lets observe the interface semantics
}

TEST_CASE( "CustomHsm - certificate and common name can be set through class interface and read by C" )
{
    auto hsm                 = std::make_shared<CustomHsm>();
    std::string expectedCert = "INVALID";

    CHECK( !hsm->setCertificate( std::vector<char>( expectedCert.begin(), expectedCert.end() ) ) );
}

TEST_CASE( "CustomHsm - certificate and common name can be set through class interface and read by C" )
{
    auto hsm                       = std::make_shared<CustomHsm>();
    std::string expectedCert       = R"(-----BEGIN CERTIFICATE-----
MIIFgDCCA2igAwIBAgIBAjANBgkqhkiG9w0BAQsFADAqMSgwJgYDVQQDDB9BenVy
ZSBJb1QgSHViIENBIENlcnQgVGVzdCBPbmx5MB4XDTIxMDcxMjA4MDMzMloXDTIx
MDgxMTA4MDMzMlowFTETMBEGA1UEAwwKdGVzdGRldmljZTCCAiIwDQYJKoZIhvcN
AQEBBQADggIPADCCAgoCggIBALHQL4FKtT21xh7fTku1LDVyJDM6C3/4FPVlZoII
lZ9ndXJVqkbZC0WwddOSPhaKPGqXby0q5OiWG0mPA+L89L/KM3kbgX4AUp+E+R++
TmtpoyyVNO6/GbXw1qRs+YRj9cBad2wf7t4mTzDopgSXjEhvDJ0Rsd5ABrALQVLf
JmS2pScSmWF/dtLGkwjcdRuYfAntOH8vp1VjxnSS2WXbK/ljQ7ZiDALLeBN+qzOA
UGdKeFL2xwV7fpwbwINRPAo9kmOw1b2IHk3PHGbhJtvFUMJj1wAocyeGfYnJ68TV
yb1EYSdAfwfkTLAaMqyfzh3E5LuSUNBNd608rF0BGXNhM5SE/iGH2iUqLqgI4+x7
T1/nFxmajTEx+7B6FuhmRMapQ5bySYrh+LKRhHiE58NsIp2WUhCB++jzgv5WYHJE
BIiPs77D49wLO1I5KAYphGE0X89RNlKlwcd0uGlCKG2MVe+ETchvKggdxXJPONZt
DfZjWvrsZR4W0u24iPeAqiHhk7E9D7tKX/waAtbrlRiIBboMa5vbT6WPs5snshLN
XFbIRBqNf/zADXbXMMnfXTXErzujR8PoPEhEEGxKyWoAzzVmC9NJa+DzY+IZbD4Q
jR7Yu58Ny26OmfH+TpZAl14E4DzeeUaN9mluh+roP4lUyrjzgy/1fV1V8EN4zGA+
T9txAgMBAAGjgcUwgcIwCQYDVR0TBAIwADARBglghkgBhvhCAQEEBAMCBaAwMwYJ
YIZIAYb4QgENBCYWJE9wZW5TU0wgR2VuZXJhdGVkIENsaWVudCBDZXJ0aWZpY2F0
ZTAdBgNVHQ4EFgQUgT8qOnygkUfMs1FcoZViqtTjMcswHwYDVR0jBBgwFoAUvxf2
UZMfhe5sAHBbumvaE66utnowDgYDVR0PAQH/BAQDAgXgMB0GA1UdJQQWMBQGCCsG
AQUFBwMCBggrBgEFBQcDBDANBgkqhkiG9w0BAQsFAAOCAgEACuaXJBYMJSaTj8IR
3kgPb6mfadyqk8jef3KE3YWEPwpthENnoft41WRgjTDvvZKNBp7ayKr8SMbh9sax
wQ+gNNhf7f3jwGM+smmHsyA0a1uJE7IiCxV8XmfDmxNTzoCJ7Aj4Dmc78f1MynQv
zuWzVSWv+9O+UCQh2IYqoDTvFOiJwwZBi+tFwcgziKs+YsUtSMai+FiY/L4WJPMx
8jrvcGYt+Z1uKrm9Hh2VNxFxLRcL+JetiRS4Kc++q/bLAV4thbiMDKREo+2h0tqj
ONL3erkrV1k6rLAqdxYRN6cMAC55w8btv9E5Wqij2uXRbf8PcQxn3gYeNK3MTjWr
48hXrPWRn50wSzzJnrJWG9bbJPm4zImbMooR57Fj/DaQ07MaW5ozg49skd2LT5Gk
3eaDel0o2vhxClOMC+OQZGhQhR+YOgMYBfMuog71xd/Iu+K6q+n0in/yreP9PEvp
FQcSXCyB7fWaqANlwyCt7qLhllhPLuKfkvMm5eg6scYiR6aDgLt/l2ApwR2f47zv
LV+E9bzonf6D0iYbaq3NooYpsRhNtkIgRPbyAwGL9t9Yke2WeWRdg+bFZlBM2tkV
Qor8xBzZ04FAk268pmiN5jhMjlskWCmFYDvV3hqyvvs/2/D8HcD2BCMozQrCv1RT
EcmUuERFUQWlKpB1nCvKQ96dyec=
-----END CERTIFICATE-----

)";
    std::string expectedCommonName = "testdevice";
    CHECK( hsm->setCertificate( std::vector<char>( expectedCert.begin(), expectedCert.end() ) ) );

    auto cInterface = hsm_client_x509_interface();

    REQUIRE( cInterface != NULL );

    auto instance = cInterface->hsm_client_x509_create(); // this does nothing, but lets observe the interface semantics

    auto cert       = cInterface->hsm_client_get_cert( instance );
    auto commonName = cInterface->hsm_client_get_common_name( instance );

    REQUIRE( cert );
    REQUIRE( commonName );

    CHECK( expectedCert == std::string( cert, cert + strlen( cert ) ) );
    CHECK( expectedCommonName == std::string( commonName, commonName + strlen( commonName ) ) );

    free( cert );
    free( commonName );

    cInterface->hsm_client_x509_destroy( instance ); // this does nothing, but lets observe the interface semantics
}

TEST_CASE( "CustomHsm - key can be set through class interface and read by C" )
{
    auto hsm                = std::make_shared<CustomHsm>();
    std::string expectedKey = R"(-----BEGIN RSA PRIVATE KEY-----
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
-----END RSA PRIVATE KEY-----

)";
    CHECK( hsm->setKey( std::vector<char>( expectedKey.begin(), expectedKey.end() ) ) );

    auto cInterface = hsm_client_x509_interface();

    REQUIRE( cInterface != NULL );

    auto instance = cInterface->hsm_client_x509_create(); // this does nothing, but lets observe the interface semantics

    auto key = cInterface->hsm_client_get_key( instance );

    REQUIRE( key );

    CHECK( expectedKey == std::string( key, key + strlen( key ) ) );

    free( key );

    cInterface->hsm_client_x509_destroy( instance ); // this does nothing, but lets observe the interface semantics
}

TEST_CASE( "CustomHsm - certificate without common name is an error" )
{
    auto hsm                 = std::make_shared<CustomHsm>();
    std::string expectedCert = R"(-----BEGIN CERTIFICATE-----
MIIChzCCAW8CFBzfA5aYC8KLJuQ56x9wmx/r+73cMA0GCSqGSIb3DQEBCwUAMAAw
HhcNMjEwNzI2MTMyNjAxWhcNMjIwNzIxMTMyNjAxWjAAMIIBIjANBgkqhkiG9w0B
AQEFAAOCAQ8AMIIBCgKCAQEAs0W4NSe+ntrtQKFjEykOfUXg+F86m809wCwXgM+r
w//SFa+8ipQTYo/rll1lCnoC54eM+NgNOqQkS9fLtEnDgFJXOCQIzN4tGaaFdCZD
Nj+//gpM9Pf//P8BIT5gP6UVwA1m/0sDUFXuZnVjibd4XQMWSm6x/LUMuOK8iunb
hphcTAjLBCvINrboQ8HcPdGsXJh8eNWqaoBP1pG1/0AhbKTaXZBuGKfLNCW3nU90
Ru3vCsBvaXmyQI3W2FBBVNhDGEAptVPJk2BVGPHL+ZEv8HbndniBaswQ02/6IVNb
2H9W/PaZRe5X3WME9U2pVPA0Wk/OHZT07yUNEjNFT/5eMQIDAQABMA0GCSqGSIb3
DQEBCwUAA4IBAQCUHYXDP6uYSqnLJlrB388iwvi2//SCafZA+mcPduRCLdctXCVt
47wMWkj9sZi+mkOnCOQN1qAzyRPxQ1ixdc/dm+eofM0sK+qLbvl43xFcpa/bdIwF
Arrn+jw4GGbMH8O6sbQC/iG9IqA86gTSqJDTBsPSdIVP+mcVrphE5LQtOQTqbWlw
EEzrf1fs4SP56OT/BaxaqPxAD6wbo5X7eNZCEgye7BxBb2CfR4JDwO+AY5UsA1aH
aA34R56XE92Mt+6vzBeWh8IaaAm7VRwZ0BnB7Fy4obylBPOAI1sqZ4LQWtpYws4w
ih2t7Bs4FNzuO/r7BxrjPJmFzD+cDvUDBNF+
-----END CERTIFICATE-----

)";
    CHECK( !hsm->setCertificate( std::vector<char>( expectedCert.begin(), expectedCert.end() ) ) );
}
