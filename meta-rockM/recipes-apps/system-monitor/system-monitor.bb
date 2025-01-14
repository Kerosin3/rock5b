SUMMARY = "system moniting app"
DESCRIPTION = "shows system workloa"
PV = "1.0"
PR = "r1"

LICENSE = "CLOSED"

SERVICE_NAME =  "system-monitor.service"
# sources
SRC_DIR = "system-monitor"

SRC_URI += " \
        file://${SRC_DIR} \
        file://${SERVICE_NAME} \
        "

APP_NAME =  "system-monitor"

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

#add systemd unit
SYSTEMD_SERVICE:${PN} = "${SERVICE_NAME} "
SYSTEMD_AUTO_ENABLE:${PN} = "disable"

do_install:append() {
  install -d ${D}/${systemd_unitdir}/system
  install -m 0644 ${WORKDIR}/${SERVICE_NAME} ${D}/${systemd_unitdir}/system
}

FILES:${PN} += "${systemd_unitdir}/system/${SERVICE_NAME}"

