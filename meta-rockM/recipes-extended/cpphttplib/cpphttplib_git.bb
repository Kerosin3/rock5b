SUMMARY = "cpp-httplib"
DESCRIPTION = "A C++11 single-file header-only cross platform HTTP/HTTPS library."

LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"


DEPENDS += " \
        "

S = "${WORKDIR}/git"

inherit pkgconfig meson

include cpphttplib-rev.inc

BBCLASSEXTEND += "native nativesdk"
