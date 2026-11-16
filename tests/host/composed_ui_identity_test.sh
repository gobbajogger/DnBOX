#!/bin/sh
# Authenticate the retained R22 source oracle before executing the actual UI fixture.
set -eu
cd "$(dirname "$0")/../.."
python3 - <<'PY'
import hashlib,json
from pathlib import Path
p=Path('tests/host/reference/ui_r22_d13_header_reference.inc')
expected='328be8b7a5a9b3a0f09059509daedaf0d3d90ef31b8be965bb004d69a942d640'
receipt_bytes=p.with_suffix('.json').read_bytes()
if hashlib.sha256(receipt_bytes).hexdigest()!='87034b657a1adab04d2d787f8c88b99d3f81276e335249794cd40b7250bbb8d6':
 raise SystemExit('R22 header reference authentication failed')
receipt=json.loads(receipt_bytes)
if receipt['sourceCommit']!='d13f410936596b6931e574e6418e35ea67af3368' or receipt['referenceSHA256']!=expected or hashlib.sha256(p.read_bytes()).hexdigest()!=expected:
 raise SystemExit('R22 header reference authentication failed')
PY
SMC_UI_TEST_FILE=tests/host/composed_ui_identity_test.c SMC_UI_OUT=${SMC_UI_OUT:-build/host/composed-ui} sh tests/host/smc_ui_test.sh
