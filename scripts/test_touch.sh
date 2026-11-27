
#!/bin/bash

cd "$(dirname "$0")/.."

./vfs-emulator \
    --vfs tests/vfs_many \
    --prompt "TOUCH:~$ " \
    --script tests/touch_startup.txt
