#!/usr/bin/env bash
# Apply local OpenWrt feed patches and drop upstream-merged feed-internal patches.
# Run inside the build container with the OpenWrt source root as cwd.
set -euo pipefail

SCRIPT_DIR=$(dirname "$(readlink -f "$0")")
PATCHES="${patches:-$SCRIPT_DIR/patches}"
FEED_OVERLAY="$SCRIPT_DIR/feed-patches"

apply_loop() {
    local feed=$1 ext=$2
    local applied=0
    ( cd "feeds/$feed" && for f in "$PATCHES"/*.$ext; do
        [ -e "$f" ] || continue
        echo "feeds/$feed: applying $(basename "$f")"
        patch -p1 < "$f"
        applied=$((applied + 1))
    done; echo "feeds/$feed: $applied patch(es) applied" )
}

apply_loop packages patch-packages
apply_loop rsu       patch-rsu

# Overlay our package-internal patches into the feed tree. Each file under
# feed-patches/<feed>/<package>/patches/ is copied to the matching
# feeds/<feed>/<package>/patches/ location, overwriting any existing patch of
# the same name. Each overlay patch carries its rationale as a leading comment
# block; patch(1) and OpenWrt's quilt loop both tolerate prose before the
# first --- header.
if [ -d "$FEED_OVERLAY" ]; then
    placed=0
    while IFS= read -r -d '' src; do
        rel="${src#$FEED_OVERLAY/}"
        dst="feeds/$rel"
        install -D -m 0644 "$src" "$dst"
        echo "placed feeds/$rel"
        placed=$((placed + 1))
    done < <(find "$FEED_OVERLAY" -type f -print0)
    echo "feed-patches overlay: $placed file(s) placed"
fi

# Drop upstream-merged feed-internal patches whose pre-images no longer match
# the LTS_03_2025 submodule pins set by
# 0035-azure-iot-sdk-c-bump-LTS_03_2025.patch-rsu.
#
# 001-fix-redirection-of-error-header.patch: swaps <sys/errno.h> for the
# POSIX <errno.h> in pal/linux/socket_async_os.h. Microsoft merged the same
# change upstream at c-utility SHA 772a4f8b (the LTS_03_2025 c-utility
# submodule), so the patch's pre-image no longer matches and the c-utility
# build's quilt loop fails with "Hunk #1 FAILED at 10".
rm -fv feeds/rsu/libs/azure-c-shared-utility/patches/001-fix-redirection-of-error-header.patch

# 001-CMakeList-change-lib-parsion-include.patch (typo: "parsion" = parson):
# swapped a bundled `add_library(parson ./deps/parson/parson.c)` for
# `find_package(parson REQUIRED CONFIG)` in azure-iot-sdk-c/CMakeLists.txt and
# removed the corresponding install rules. Microsoft merged the same change
# upstream at azure-iot-sdk-c LTS_03_2025 (CMakeLists.txt line ~188), so the
# patch's hunks no longer apply (Hunk #1 FAILED at 181, Hunk #2 FAILED at 422).
# The intent is preserved by the overlay's
# 004-CMakeLists-drop-top-level-deps-subdir.patch.
rm -fv feeds/rsu/libs/azure-iot-sdk-c/patches/001-CMakeList-change-lib-parsion-include.patch
