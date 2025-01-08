SUMMARY = "device simulator"
DESCRIPTION = "to test"
PV = "1.0"
PR = "r1"

LICENSE = "CLOSED"

#SERVICE_NAME =  "server.service"
# sources
SRC_DIR = "device-simulator"

SRC_URI += " \
        file://${SRC_DIR} \
        "

APP_NAME =  "device-simulator"

DEPENDS += "libgpiod"
DEPENDS += "cli11"
DEPENDS += "systemd"
DEPENDS += "sdbusplus"

inherit meson pkgconfig systemd

S = "${WORKDIR}/${SRC_DIR}"

BUILD_APP1 = "${WORKDIR}/build/${APP_NAME}"

#add server binary
do_install(){
	install -d ${D}${bindir}
	install -m 0755 ${BUILD_APP1} ${D}${bindir}
}

FILES:${PN} += " \
               ${bindir} \
               "
