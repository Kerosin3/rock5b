KBRANCH ?= "v6.10"
LINUX_VERSION ?= "6.10.3"
PV = "${LINUX_VERSION}+git${SRCPV}"

SRCREV="0c3836482481200ead7b416ca80c68a29cfdaabd"

SRC_URI:append = " file://rockchip-kmeta;type=kmeta;name=rockchip-kmeta;destsuffix=rockchip-kmeta"

require linux-rockchipx.inc

