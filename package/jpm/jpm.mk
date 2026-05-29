JPM_VERSION = 1.0.0
JPM_SITE = https://gitlab.com/jinaos/jpm
JPM_SITE_METHOD = git
JPM_LICENSE = MIT

define JPM_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/jpm $(TARGET_DIR)/usr/bin/jpm
	mkdir -p $(TARGET_DIR)/var/cache/jpm
endef

$(eval $(generic-package))