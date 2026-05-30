#!bin/sh
PASSWORD_HASH_$(openssl passwd -6 -salt $(openssl rand -base64 12) jinaos123)
sed -i "s|jinaos:.*:|jinaos:$PASSWORD_HASH_jinaos:|" ${TARGET_DIR}/etc/shadow
