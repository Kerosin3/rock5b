SUMMARY = "fan fix service"
PV = "1.0"
PR = "r1"

LICENSE = "CLOSED"

SERVICE_NAME =  "fan-fix.service"
# sources
SRC_URI += " \
        file://${SERVICE_NAME} \
        "

DEPENDS += "systemd"

inherit systemd

SYSTEMD_SERVICE:${PN} = "${SERVICE_NAME} "
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_install:append() {
  install -d ${D}/${systemd_unitdir}/system
  install -m 0644 ${WORKDIR}/${SERVICE_NAME} ${D}/${systemd_unitdir}/system
}

FILES:${PN} += "${systemd_unitdir}/system/${SERVICE_NAME}"

