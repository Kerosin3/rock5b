#!/bin/bash

IP="192.168.0.101"

echo ip is $IP

execute_remote() {
  sshpass -p 0penBmc ssh -q -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null root@${IP} $1 2>/dev/null
}


file1="./build/setting-manager"

#arm-openbmc-linux-gnueabi-strip $file1

#scp ./build/clientapp root@$IP:/usr/bin
rsync -avziHPL -e "sshpass -p root ssh -q -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null" $file1 root@$IP:/usr/bin/setting-manager
      if [[ $? -ne 0 ]]; then
        echo error while transferring file to bmc
        exit 1
      fi
      
echo "[DONE]"
