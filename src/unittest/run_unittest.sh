#!/usr/bin/env bash
TOOLCHAIN_LIB=/openwrt/staging_dir/toolchain-x86_64_gcc-8.4.0_musl/lib/
TARGET_USR_LIB=/openwrt/staging_dir/target-x86_64_musl/usr/lib/
mkdir -p buildTest
cd buildTest
cat << EOT > valid_config.json
{
  "stringitem": "testsetting",
  "intitem": 99999,
  "boolitem": true
}
EOT
${TOOLCHAIN_LIB}ld-musl-x86_64.so.1 --library-path=${TOOLCHAIN_LIB}:${TARGET_USR_LIB}:${TARGET_USR_LIB}/oatpp-1.2.5/ ../buildX64/rsu-connector/UT_rsu-connector $@
cd ..

