SUMMARY = "device simulator application and service"
DESCRIPTION = "device simulator example"
PV = "1.0"
PR = "r1"

LICENSE = "CLOSED"

SERVICE_NAME =  "device-simulator.service"
# sources
SRC_DIR = "device-simulator"

SRC_URI += " \
        file://${SRC_DIR} \
        file://${SERVICE_NAME} \
        "

APP_NAME =  "device-simulator"

DEPENDS += "libgpiod"
DEPENDS += "cli11"
DEPENDS += "systemd"
DEPENDS += "sdbusplus"
DEPENDS += "libcereal"

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
#add systemd unit
SYSTEMD_SERVICE:${PN} = "${SERVICE_NAME} "
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_install:append() {
  install -d ${D}/${systemd_unitdir}/system
  install -m 0644 ${WORKDIR}/${SERVICE_NAME} ${D}/${systemd_unitdir}/system
}

FILES:${PN} += "${systemd_unitdir}/system/${SERVICE_NAME}"
