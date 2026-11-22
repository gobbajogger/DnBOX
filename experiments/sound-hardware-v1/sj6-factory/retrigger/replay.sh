#!/bin/sh
# Build the original-derived comparator outside the firmware sources.
set -eu
if [ "$#" -ne 2 ]; then
    echo "usage: sh replay.sh GIT_CHECKOUT FRESH_OUTPUT_DIRECTORY" >&2
    exit 2
fi
here=$(cd "$(dirname "$0")" && pwd)
ref=4ea86b9e869011cae761709d13569b7bc32980a8
pkg=experiments/superj6-original-reference-20261115
mkdir -- "$2"
out=$(cd "$2" && pwd)
git -C "$1" archive --output="$out/reference.tar" "$ref" -- "$pkg/sampler"
tar -xf "$out/reference.tar" -C "$out"
sampler="$out/$pkg/sampler"
common="$sampler/DunneCore/Common"
voices="$sampler/DunneCore/Sampler"
cc -std=c11 -O2 -ffp-contract=off -fno-fast-math -I"$common" \
    -c "$common/numeric_boundary.c" -o "$out/numeric.o"
c++ -std=c++17 -O2 -ffp-contract=off -fno-fast-math \
    -I"$common" -I"$voices" -I"$sampler/include" "$here/reference.cpp" \
    "$common"/*.cpp "$voices"/*.cpp "$out/numeric.o" -o "$out/reference"
"$out/reference"
