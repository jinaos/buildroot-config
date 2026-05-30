#!/bin/bash

cd board/jinaos/src/gesture-daemon
cargo build --release --target aarch64-unknown-linux-musl
strip target/aarch64-unknown-linux-musl/release/gesture-daemon
cp target/aarch64-unknown-linux-musl/release/gesture-daemon ../../rootfs_overlay/usr/bin/