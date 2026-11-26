#!/bin/bash

cd "$(dirname "$0")/.."

./vfs-emulator \
    --vfs tests/vfs_empty \
    --prompt "EMPTY:~$ "