# How to Build the RSU Connector

[[_TOC_]] 

---

## Prepare the build environment

The RSU connector can be built in Ubuntu 18.04 OS. The example below shows how a docker environment can be used for the build process.
The following steps assume this repository is checked out in the current directory.



#### Make sure external submodules are present

```bash
git submodule update --init --recursive
```

#### Docker file

Refer to [Dockerfile](./../Dockerfile) for setting up the build environment.

#### Build the container image

``` bash
docker build -t rsu-connector-build:latest .
```

#### Preparation for the build

To be able to resume a build, especially of OpenWRT which takes a lot of time, it is recommended to mount a volume into the container where the build artefacts will reside.
If you do the build directly in the container, the results will be lost when it is shut down.

``` bash
mkdir ../openwrt
# The user id of the user in the container will likely not match the current user id, so make sure the container can write there.
chmod a+rwx ../openwrt
```

#### Run the container image

This will mount the openwrt directory just created into /openwrt in the container.

``` bash
docker run -it -v $(pwd)/../openwrt:/openwrt rsu-connector-build:latest /bin/bash
```

All the following commands should now be running inside the container.

---


## Build the OpenWRT

Since some of the modules in the rsu-connector depend on libraries supplied by OpenWRT, we need to have an OpenWRT build present.
The next steps show how to do this. The variables used here are defined in the Dockerfile.

#### Clone the OpenWRT repository

``` bash

git clone $openwrtSource $openwrtDir
cd $openwrtDir
# since we are mounting the /openwrt from outside, the permissions may be suspicious for git
git config --global --add safe.directory /openwrt
git checkout $openwrtCommitId

make distclean
```

#### Update and install feeds

``` bash
cd $openwrtDir

cp $srcFeeds feeds.conf

# Update the feeds, throwing away changes
./scripts/feeds update -a -f

# Applying patches to packages feed to backport spdlog and libfmt.
( cd feeds/packages; for f in $patches/*.patch; do patch -p1 < $f; done )

# Applying patches to rsu feed to fix issue with missing -fPIC.
( cd feeds/rsu; for f in $patches/*.patch-rsu; do patch -p1 < $f; done )

# Update feeds index to pick up patches.
./scripts/feeds update -a -i
./scripts/feeds install -a
```

#### Build OpenWRT for either x86-64 or mips

Configurations are provided for building OpenWRT for a mips based architecture and for x86-64.
You likely will need to only build for one of the architectures.

``` bash
cd $openwrtDir

# Building OpenWrt for mips.
cp $srcConfigMips .config
make defconfig
make download
make -j5 world
make package/feeds/rsu/rapidjson/compile
make package/feeds/packages/spdlog/compile
make package/feeds/rsu/cpp-base64/compile

# Building OpenWrt for x86_64.
cp $srcConfigX86 .config
make defconfig
make download
make -j5 world
make package/feeds/rsu/rapidjson/compile
make package/feeds/packages/spdlog/compile
make package/feeds/rsu/cpp-base64/compile
```

---

## How to Build the Remote Service Unit Connector Application

#### Build Mips binary

``` bash
cd $rsuConnectorSourceDir
./build_mips.sh -j5
```

#### Build x86_64 binary

``` bash
cd $rsuConnectorSourceDir
./build_x86.sh -j5
```

#### Run unit tests

Needs x86_64 build.

``` bash
cd $rsuConnectorSourceDir
chmod a+x ./unittest/*.sh
./unittest/run_unittest.sh --out=./unittest/unittests.junit --reporters=junit
./unittest/generate_coverage.sh
```
