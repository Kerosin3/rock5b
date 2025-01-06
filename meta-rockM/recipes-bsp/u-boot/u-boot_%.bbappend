FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += " \
    file://${MACHINE}.cfg \
    file://rk3588-rock-5b-u-boot.dtsi;subdir=git/arch/arm/dts \
    file://rk3588-u-boot.dtsi;subdir=git/arch/arm/dts \
    file://rk3588.dtsi;subdir=git/arch/arm/dts \
    file://rk3588-rock-5b.dts;subdir=git/arch/arm/dts \
"


