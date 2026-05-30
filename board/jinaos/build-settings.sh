#!/bin/bash

# Compile settings app for ARM64
aarch64-linux-gcc -o settings settings.c `pkg-config --cflags --libs gtk+-3.0` -static

# Strip binary
strip settings

# Copy to rootfs overlay
cp settings ../rootfs_overlay/usr/bin/jinaos-settings