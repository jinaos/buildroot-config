
#!/bin/sh

# Set jinaos user password
PASSWORD_HASH=$(openssl passwd -6 -salt $(openssl rand -base64 12) jinaos123)

# Replace password in shadow file
sed -i "s|jinaos:.*:|jinaos:${PASSWORD_HASH}:|" ${TARGET_DIR}/etc/shadow

# Ensure init scripts are executable
chmod +x ${TARGET_DIR}/etc/init.d/S20jpm-cache
chmod +x ${TARGET_DIR}/etc/init.d/S40iptables
chmod +x ${TARGET_DIR}/etc/init.d/S45hostapd
chmod +x ${TARGET_DIR}/etc/init.d/S50weston

# Ensure sudoers file has correct permissions
chmod 440 ${TARGET_DIR}/etc/sudoers.d/jinaos

# Create runtime directory for Wayland
mkdir -p ${TARGET_DIR}/tmp/runtime-jinaos
# Compile C settings app
cd ${BR2_EXTERNAL}/board/jinaos/src
./build-settings.sh
chmod +x ${TARGET_DIR}/etc/init.d/S55swaync

exit 0