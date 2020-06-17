#!/usr/bin/env bash

# Run a command and output a line of text every second until the command
# completes. This script works around a Travis bug on Linux for aarch64, where
# an unusually large amount of text must be outputted to the log to prevent
# Travis from thinking that the build has hung.
#
# Usage: ./spam_build_log.sh COMMAND ARGS

"$@" &
PID=$!

while kill -0 "$PID"; do
  echo "(This message is intended to flood the build log in order to work around a Travis bug on Linux for aarch64.)"
  sleep 1
done
