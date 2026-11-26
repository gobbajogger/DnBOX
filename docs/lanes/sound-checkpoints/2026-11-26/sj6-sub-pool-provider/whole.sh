#!/bin/bash
set -euo pipefail
D=$(cd "$(dirname "$0")"&&pwd)
R=$(dirname "$D")
cd "$R"
# Reuse the preceding isolated host build wrappers, with only the scorer changed.
sed 's@python3 "$D/score.py" "$O" "$B"@python3 "$D/../sj6-sub-pool-provider/whole.py" "$O" "$B"@' sj6-postvoice-split/run.sh > sj6-postvoice-split/pool-run.temporary.sh
trap 'rm -f sj6-postvoice-split/pool-run.temporary.sh' EXIT
bash sj6-postvoice-split/pool-run.temporary.sh
