#!/bin/sh
# Dispatch to the named binary passed as first argument, or default to example-client.
case "$1" in
  example-*)
    BIN="$1"; shift; exec "/usr/local/bin/$BIN" "$@" ;;
  *)
    exec /usr/local/bin/example-client "$@" ;;
esac
