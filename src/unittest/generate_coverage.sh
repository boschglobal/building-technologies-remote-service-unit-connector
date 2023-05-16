#!/bin/sh
rm -rf coverage
mkdir -p coverage
gcovr -r . --gcov-executable /openwrt/staging_dir/toolchain-x86_64_gcc-8.4.0_musl/bin/x86_64-openwrt-linux-musl-gcov --exclude 'external/.*' --exclude 'rsu-connector/unittest/.*' --html-details coverage/rsu-connector.html --xml --xml-pretty > coverage/rsu-connector.xml

