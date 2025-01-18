SUMMARY = "dbus setup application"
DESCRIPTION = "service that expose setup to dbus"
PV = "1.0"
PR = "r1"

LICENSE = "CLOSED"

DBUS_CONF_NAME =  "setups.conf"
SERVICE_NAME =  "setups.service"
# sources
SRC_DIR = "app"
SRC_URI += " \
        file://${SRC_DIR} \
        file://${DBUS_CONF_NAME} \
        file://${SERVICE_NAME} \
        "

APP_NAME =  "setups"

DEPENDS += "libgpiod"
DEPENDS += "cli11"
DEPENDS += "systemd"
DEPENDS += "sdbus-c++"
DEPENDS += "sdbusplus"

inherit meson pkgconfig systemd

S = "${WORKDIR}/${SRC_DIR}"

BUILD_APP1 = "${WORKDIR}/build/${APP_NAME}"

do_install(){
	install -d ${D}${bindir}
    install -d ${D}${sysconfdir}/dbus-1/system.d
    install -m 0755 ${WORKDIR}/${DBUS_CONF_NAME} ${D}${sysconfdir}/dbus-1/system.d/
	install -m 0755 ${BUILD_APP1} ${D}${bindir}
}

FILES:${PN} += " \
               ${bindir} \
               ${sysconfdir}/dbus-1/system.d \
               ${sysconfdir}/dbus-1/system.d/${DBUS_CONF_NAME} \
               "

#add systemd unit
SYSTEMD_SERVICE:${PN} = "${SERVICE_NAME} "
SYSTEMD_AUTO_ENABLE:${PN} = "enable"

do_install:append() {
  install -d ${D}/${systemd_unitdir}/system
  install -m 0644 ${WORKDIR}/${SERVICE_NAME} ${D}/${systemd_unitdir}/system
}

FILES:${PN} += "${systemd_unitdir}/system/${SERVICE_NAME}"

