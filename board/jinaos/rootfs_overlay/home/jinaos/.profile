#!/bin/sh

if [ -z "$WAYLAND_DISPLAY" ]; then
	export WESTON_STARTED=1
	echo "starting JinaOS GUI..."
	exec weston --config=/etc/weston.ini --tty=1
fi