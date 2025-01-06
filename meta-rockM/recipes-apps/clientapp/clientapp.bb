SUMMARY = "test project"
DESCRIPTION = "to test"
PV = "1.0"
PR = "r1"

# License info
LICENSE = "CLOSED"

SRC_DIR = "app"

SRC_URI += "file://${SRC_DIR}"

APP_NAME =  "clientapp"

DEPENDS += "libgpiod"
DEPENDS += "cli11"
DEPENDS += "systemd"
DEPENDS += "sdbus-c++"
DEPENDS += "crow"
DEPENDS += "openssl"
DEPENDS += "sdbusplus"

inherit meson pkgconfig

S = "${WORKDIR}/${SRC_DIR}"

BUILD_APP1 = "${WORKDIR}/build/${APP_NAME}"

do_install(){
	install -d ${D}${bindir}
	install -m 0755 ${BUILD_APP1} ${D}${bindir}
}
