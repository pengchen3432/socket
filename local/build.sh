#!/bin/bash

. ../../../public/build_funcs.sh

case "$1" in
    "gwn7062")
        PRODUCT="-DGWN7062 -DUSE_CFG80211 -DDOUBLE_WAN"
        BNDSTR="LBD"
        ;;
    "gwn7052")
        PRODUCT="-DGWN7052 -DCONFIG_MTK -DSINGLE_WAN"
        BNDSTR="BNDSTRG"
        ;;
    *)
        echo "Unrecognized Product!"
        exit
        ;;
esac

[ -n "${PLATFORM}" ] && ARCH=${PLATFORM}
GSFLAGS="$PRODUCT"

make clean
make CROSS=${CROSS_PREFIX}  GSFLAGS="$GSFLAGS" BNDSTR="${BNDSTR}"
if [ "$?" != 0 ]; then
    exit
fi

