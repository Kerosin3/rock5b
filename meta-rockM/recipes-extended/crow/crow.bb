SUMMARY = "Simple test application"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit pkgconfig cmake

SRC_URI = "git://github.com/CrowCpp/Crow;protocol=https;branch=900-dead-link-on-crow-documentation-crow1.0"
#SRCREV = "${AUTOREV}"
SRCREV = "edf12f699ec3bf6f751cf73cb97f32919e48ca6e"
BB_STRICT_CHECKSUM = "0"

S = "${WORKDIR}/git"

DEPENDS += "boost "
DEPENDS += "asio "

EXTRA_OECMAKE += "-DCROW_INSTALL=ON"
EXTRA_OECMAKE += "-DCROW_ENABLE_SSL=TRUE"
EXTRA_OECMAKE += "-DCROW_BUILD_TESTS=FALSE"
EXTRA_OECMAKE += "-DCROW_BUILD_EXAMPLES=FALSE"
TARGET_CFLAGS += "-Wno-unused-result"
TARGET_CFLAGS += "-Wno-address"
TARGET_CFLAGS += "-Wno-type-limits"

BBCLASSEXTEND += "native nativesdk"
TOOLCHAIN_TARGET_TASK:append = " crow "