FILESEXTRAPATHS:prepend := "${THISDIR}/${PN}:"

SRC_URI += " \
    file://${MACHINE}.cfg \
    file://rk3588-rock-5b.dts \
    file://rk3588.dtsi \
"

do_patch:append() {
  for DTB in "${KERNEL_DEVICETREE}"; do
      DT=`basename ${DTB} .dtb`
      if [ -r "${WORKDIR}/${DT}.dts" ]; then
          cp ${WORKDIR}/rk3588-rock-5b.dts \
              ${STAGING_KERNEL_DIR}/arch/${ARCH}/boot/dts/rockchip
          cp ${WORKDIR}/rk3588.dtsi \
             ${STAGING_KERNEL_DIR}/arch/${ARCH}/boot/dts/rockchip
      fi
  done 
}
