#!/bin/sh
set -eu

# VFS path + custom prompt, without a startup script.
printf 'conf-dump\nexit\n' | ./emulator \
  --vfs "./test-data/vfs" \
  --prompt "vfs-test:~$ "

# All supported parameters, with a different startup script path.
printf 'exit\n' | ./emulator \
  --vfs "./test-data/another-vfs" \
  --prompt "second-test:~$ " \
  --script "./scripts/start.txt"
