#!/bin/sh
set -eu
exec sh "$(dirname "$0")/replay-old.sh" "$@"
