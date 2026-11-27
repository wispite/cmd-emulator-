#!/bin/bash

cd "$(dirname "$0")/.."

./vfs-emulator \
    --vfs tests/vfs_many \
    --prompt "STARTUP:~$ " \
    --script tests/startup.txt