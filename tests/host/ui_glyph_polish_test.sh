#!/bin/sh
# SPDX-License-Identifier: GPL-3.0-only
set -eu
cd "$(dirname "$0")/../.."
python3 tests/host/ui_glyph_caption_contract_test.py
