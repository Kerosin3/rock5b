DESCRIPTION = "Linux kernel for rockchip"
SECTION = "kernel"
LICENSE = "GPL-2.0-only"

PROVIDES += "virtual/kernel"

#KCONFIG_MODE="--alldefconfig"

inherit kernel
require recipes-kernel/linux/linux-yocto.inc

KSRC ?= "git://github.com/armbian/linux-rockchip;protocol=https;branch=${KBRANCH}"

#unset KBUILD_DEFCONFIG
KBUILD_DEFCONFIG = "rockchip_linux_defconfig"
#KERNEL_DEVICETREE:append = " rockchip/overlay/rk3588-i2c7-m3.dtbo"

SRC_URI += "${KSRC}"


SRC_URI += " \
    file://rock-5bx.cfg \
    file://rk3588-rock-5b.dts \
    file://rk3588.dtsi \
    file://rk3588-vccio3-pinctrl.dtsi \
"

do_patch:append() {
  for DTB in "${KERNEL_DEVICETREE}"; do
      DT=`basename ${DTB} .dtb`
      if [ -r "${WORKDIR}/${DT}.dts" ]; then
          cp ${WORKDIR}/rk3588-rock-5b.dts \
             ${STAGING_KERNEL_DIR}/arch/${ARCH}/boot/dts/rockchip
          cp ${WORKDIR}/rk3588.dtsi \
             ${STAGING_KERNEL_DIR}/arch/${ARCH}/boot/dts/rockchip
          cp ${WORKDIR}/rk3588-vccio3-pinctrl.dtsi \
             ${STAGING_KERNEL_DIR}/arch/${ARCH}/boot/dts/rockchip

      fi
  done 
}

LINUX_VERSION_EXTENSION ?= "-${SRCREV}"

PV = "${LINUX_VERSION}+git${SRCPV}"
# From 5.6+ the COPYING file changed
LIC_FILES_CHKSUM = "file://COPYING;md5=6bc538ed5bd9a7fc9398086aedcd7e46"
