#!/bin/sh
set -eu

# Test all three command-line parameters together and execute a startup script.
printf 'conf-dump\nexit\n' | ./emulator \
  --vfs "./test-data/vfs" \
  --prompt "test-user@host:~$ " \
  --script "./scripts/start.txt"
