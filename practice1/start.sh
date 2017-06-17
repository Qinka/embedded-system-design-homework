#!/bin/bash

module="ledmod"
device="qled"
mode="0666"
set -e

/sbin/insmode ./$module.ko $* || exit 1

rm -f /dev/${device}

major=$(awk "\$2=   =\"$module\" {print \$1}" /rpc/devices)
mknod /dev/${device} c $major 0
chmod $mode /dev/${device}
./daemon
