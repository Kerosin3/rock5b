SUMMARY = "Stargazer development image"

inherit core-image

EXTRA_IMAGE_FEATURES += " \
    ssh-server-openssh \
    tools-debug \
    "

CORE_IMAGE_EXTRA_INSTALL += " \
    ethtool \
    evtest \
    fbset \
    i2c-tools \
    memtester \
    "


#EXTRA_IMAGE_FEATURES = " debug-tweaks tools-debug empty-root-password allow-empty-password allow-root-login tools-profile serial-autologin-root "
EXTRA_IMAGE_FEATURES += " \
    debug-tweaks \
    tools-debug \
    empty-root-password \
    allow-empty-password \
    allow-root-login \
    tools-profile \
    "

IMAGE_INSTALL:append = " \
    rsync \
    net-tools \
    mingetty \
    git \
    sdbusplus \
    vim \
    python3-flask \
    libgpiod \
    devmem2 \
    libcereal \
    asio \
    fan-fix \
    nlohmann-json \
    webapp \
    server-dbus \
    setups-dbus \
    device-simulator \
    system-monitor \
"

inherit extrausers
#IMAGE_FEATURES:remove = "debug-tweaks"

PASSWD1 = "\$5\$e8q5Pkpokl3lTO2O\$/nNdgQD.GvzcRp/aEswJeoNnriwW9yoIwKRySZiIwE3"
PASSWD2 = "\$5\$Cj3yrQ3XNUBKp3dR\$GFhBHcGM9ynCEAc3nupGj1NJHLDrE.dZjcvEvDUagY8"

EXTRA_USERS_PARAMS:append = "\
    useradd alex; \
    usermod -p '${PASSWD1}' alex; \
    usermod -ou 0  alex; \
    usermod -p '${PASSWD1}' root; \
    useradd -p '${PASSWD2}' test; \
    usermod -a -G sudo test; \
    "
