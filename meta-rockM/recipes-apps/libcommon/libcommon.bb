SUMMARY = "libcommon"
DESCRIPTION = "library"
PV = "1.0"
PR = "r1"

LICENSE = "CLOSED"

DBUS_CONF_NAME =  "test.conf"
SERVICE_NAME =  "main-service.service"
# sources
SRC_DIR = "meson-prj"
SRC_URI += " \
        file://${SRC_DIR} \
        "

APP_NAME =  "main-service"

DEPENDS += "systemd"

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

