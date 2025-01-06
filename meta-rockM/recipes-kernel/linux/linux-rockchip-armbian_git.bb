KBRANCH ?= "rk-6.1-rkr4.1"
LINUX_VERSION ?= "6.1.84"
PV = "${LINUX_VERSION}+git${SRCPV}"

SRCREV="6693dbcacb19dcbb851abba3bab668af351892ec"

SRC_URI:append = " file://rockchip-kmeta;type=kmeta;name=rockchip-kmeta;destsuffix=rockchip-kmeta"

require linux-rockchip-armbian.inc

