#!/bin/bash

. ../../../public/build_funcs.sh

case "$1" in
    "gwn7062")
        PRODUCT="-DGWN7062 -DUSE_CFG80211 -DDOUBLE_WAN"
        ;;
    "gwn7052")
        PRODUCT="-DGWN7052 -DCONFIG_MTK -DSINGLE_WAN"
        ;;
    *)
        echo "Unrecognized Product!"
        exit
        ;;
esac

[ -n "${PLATFORM}" ] && ARCH=${PLATFORM}
GSFLAGS="$PRODUCT"

make clean
make CROSS=${CROSS_PREFIX}  GSFLAGS="$GSFLAGS"
if [ "$?" != 0 ]; then
    exit
fi

ccd cfmanager ${IMAGE_BUILD}packages/files/cfmanager/${PRODUCT_TYPE}/${DEV}/usr/sbin/

mkdir -p ${IMAGE_BUILD}packages/files/cfmanager/${PRODUCT_TYPE}/${DEV}/etc/rc.d
mkdir -p ${IMAGE_BUILD}packages/files/cfmanager/${PRODUCT_TYPE}/${DEV}/etc/init.d
mkdir -p ${IMAGE_BUILD}packages/files/cfmanager/${PRODUCT_TYPE}/${DEV}/etc/config

cp -av cfmanager.init ${IMAGE_BUILD}packages/files/cfmanager/${PRODUCT_TYPE}/${DEV}/etc/init.d/cfmanager

cd ${IMAGE_BUILD}packages/files/cfmanager/${PRODUCT_TYPE}/${DEV}/etc/rc.d
rm -rf S*cfmanager
ln -s ../init.d/cfmanager S10cfmanager
cd -
