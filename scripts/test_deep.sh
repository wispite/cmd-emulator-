#!/bin/bash

cd "$(dirname "$0")/.."

./vfs-emulator \
    --vfs tests/vfs_deep \
    --prompt "DEEP:~$ "