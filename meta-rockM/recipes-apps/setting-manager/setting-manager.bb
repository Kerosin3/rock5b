SUMMARY = "setting manager application"
PV = "1.0"
PR = "r1"

LICENSE = "CLOSED"

CONFIG_FILE =  "config.yaml"
SERVICE_NAME =  "setting-manager.service"
# sources
SRC_DIR = "app"

SRC_URI += " \
        file://${SRC_DIR} \
        file://${CONFIG_FILE} \
        file://${SERVICE_NAME} \
        "

APP_NAME =  "setting-manager"

DEPENDS += "libgpiod"
DEPENDS += "cli11"
DEPENDS += "systemd"
DEPENDS += "sdbus-c++"
DEPENDS += "sdbusplus"
DEPENDS += "yaml-cpp"

inherit meson pkgconfig systemd

S = "${WORKDIR}/${SRC_DIR}"

BUILD_APP1 = "${WORKDIR}/build/${APP_NAME}"

do_install(){
	install -d ${D}${bindir}
    install -d ${D}${sysconfdir}
    install -m 0755 ${WORKDIR}/${CONFIG_FILE} ${D}${sysconfdir}
	install -m 0755 ${BUILD_APP1} ${D}${bindir}
}

FILES:${PN} += " \
               ${bindir} \
               ${sysconfdir}/${CONFIG_FILE} \
               "

#add systemd unit
SYSTEMD_SERVICE:${PN} = "${SERVICE_NAME} "
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_install:append() {
  install -d ${D}/${systemd_unitdir}/system
  install -m 0644 ${WORKDIR}/${SERVICE_NAME} ${D}/${systemd_unitdir}/system
}

FILES:${PN} += "${systemd_unitdir}/system/${SERVICE_NAME}"
