PHOSH_PHONE_VERSION = 1.0
PHOSH_PHONE_LICENSE = GPL-3.0+

define PHOSH_PHONE_INSTALL_TARGET_CMDS
    # No source to build, just a meta-package
endef

# Dependencies that will be pulled in
PHOSH_PHONE_DEPENDENCIES = \
    chatty \
    gnome-calls \
    gnome-contacts \
    modemmanager

$(eval $(generic-package))