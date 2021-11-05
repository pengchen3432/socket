#!/bin/bash
. ../../../../public/build_funcs.sh


echo ${PUBLIC_INCLUDE}
make clean
make CROSS=${CROSS_PREFIX}  
if [ "$?" != 0 ]; then
    exit
fi
ccd mdns ${IMAGE_BUILD}packages/files/mdns/${PRODUCT_TYPE}/${DEV}/sbin

