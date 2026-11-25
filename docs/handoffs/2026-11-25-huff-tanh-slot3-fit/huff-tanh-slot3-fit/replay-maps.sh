#!/bin/bash
set -euo pipefail
cd "$(dirname "$0")"
export D9_LAB_SLOT3_METADATA=1 DNBOX_TANH_SPLIT_LAB=1 DNBOX_TANH_REFILL_LAB=1
exec bash run.sh
