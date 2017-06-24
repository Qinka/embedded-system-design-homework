#!/bin/bash

module="ledmod"
device="qled"
mode="0666"
set -e

/sbin/insmod ./$module.ko $* || exit 1

rm -f /dev/${device}

major=`cat /proc/devices| grep qled | awk '{print $1}'`
mknod /dev/${device} c $major 0
chmod $mode /dev/${device}
#./daemon
