KBRANCH ?= "linux-6.1-stan-rkr4.1"
LINUX_VERSION ?= "6.1.84"
PV = "${LINUX_VERSION}+git${SRCPV}"

SRCREV="3bf163742624a137cf20c134721e3ac185d44108"

SRC_URI:append = " file://rockchip-kmeta;type=kmeta;name=rockchip-kmeta;destsuffix=rockchip-kmeta"

require linux-rockchip.inc

