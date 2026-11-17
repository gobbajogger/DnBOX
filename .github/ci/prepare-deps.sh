#!/bin/bash
# Fetch public CI dependencies for the firmware Local gate. Never uploads the
# toolchain or SDK. Missing items are printed as "needs X"; this script exits 0
# so the gate can still run host tests and skip only what cannot be built.
set -u
DEPS="${DNBOX_DEPS:-$HOME/dnbox-deps}"
mkdir -p "$DEPS/jieli" "$DEPS/ac79" "$DEPS/schwung"
: > "$DEPS/MISSING.txt"

ok() { echo "prepare-deps: $*"; }
need() { echo "needs $*" >&2; echo "needs $*" >> "$DEPS/MISSING.txt"; }

# --- Python packages and Playwright Chromium (lua / host browser tests) ---
python3 -m pip install --upgrade pip
if python3 -m pip install Pillow numpy playwright; then
  if python3 -m playwright install --with-deps chromium; then
    ok "Pillow, numpy, Playwright Chromium installed"
  else
    need "Playwright Chromium (playwright install --with-deps chromium failed)"
  fi
else
  need "Python packages Pillow/numpy/playwright (pip install failed)"
fi
CHROMIUM=$(python3 -c 'from playwright.sync_api import sync_playwright as s; p=s().start(); print(p.chromium.executable_path); p.stop()' 2>/dev/null || true)
if [ -n "${CHROMIUM:-}" ]; then
  if [ -n "${GITHUB_ENV:-}" ]; then
    echo "CHROMIUM_EXECUTABLE=$CHROMIUM" >> "$GITHUB_ENV"
    echo "DNBOX_CHROMIUM_EXECUTABLE=$CHROMIUM" >> "$GITHUB_ENV"
  fi
  export CHROMIUM_EXECUTABLE="$CHROMIUM"
  ok "CHROMIUM_EXECUTABLE=$CHROMIUM"
fi

# --- JieLi Linux toolchain (public package server; tools/get_toolchain.sh) ---
CLANG="$DEPS/jieli/toolchain/pi32v2/bin/clang"
if [ -x "$CLANG" ]; then
  ok "JieLi toolchain cached at $CLANG"
elif bash tools/get_toolchain.sh "$DEPS/jieli" && [ -x "$CLANG" ]; then
  ok "JieLi toolchain installed at $CLANG"
else
  need "JieLi toolchain (public download https://pkgman.jieliapp.com/s/linux-toolchain failed). Firmware Elite/Pro builds will be skipped."
fi

# --- AC79 SDK files the package actually uses (Apache-2.0, public Gitee tag).
# The full checkout is ~1.1G; tools/build.py pins these three files only.
SDK_TAG="AC79NN_SDK_V1.2.14_2026-10-13"
SDK_BASE="https://gitee.com/Jieli-Tech/fw-AC79_AIoT_SDK/raw/${SDK_TAG}/cpu/wl82/tools"
fetch_sdk() {
  local rel="$1" sha="$2"
  local dest="$DEPS/ac79/cpu/wl82/tools/$rel"
  mkdir -p "$(dirname "$dest")"
  if [ -f "$dest" ] && [ "$(sha256sum "$dest" | awk '{print $1}')" = "$sha" ]; then
    ok "SDK $rel cached and hash-ok"
    return 0
  fi
  if curl -fL --retry 5 --retry-delay 2 --max-time 120 "$SDK_BASE/$rel" -o "$dest.tmp"; then
    local got
    got=$(sha256sum "$dest.tmp" | awk '{print $1}')
    if [ "$got" = "$sha" ]; then
      mv "$dest.tmp" "$dest"
      ok "SDK $rel fetched and hash-ok"
      return 0
    fi
    echo "prepare-deps: SHA-256 mismatch for $rel (got $got, want $sha)" >&2
  fi
  rm -f "$dest.tmp"
  return 1
}
sdk_ok=1
fetch_sdk "uboot.boot" "64ccc63e4df3238046fadb200f1b134311d8fe6ec6e3ccb8c8b81379231b867e" || sdk_ok=0
fetch_sdk "cfg_tool.bin" "276579954f076886a6a7694f65dc71c034a63a2c204b76749065c0ac7b010d1b" || sdk_ok=0
fetch_sdk "cfg/eq_cfg_hw.bin" "179ac9f4501d2cf8c3bdbc06fcda1e46228b7ace7e339342c767bb6568de57d8" || sdk_ok=0
if [ "$sdk_ok" -ne 1 ]; then
  need "AC79 SDK files from public Apache-2.0 tag $SDK_TAG at https://gitee.com/Jieli-Tech/fw-AC79_AIoT_SDK. Firmware Elite/Pro builds will be skipped."
fi

# --- Public schwung reference trees. SHAs match the box reference checkouts.
fetch_schwung() {
  local url="$1" sha="$2" tag="$3" dest="$4" name="$5"
  if [ -d "$dest/.git" ]; then
    local have
    have=$(git -C "$dest" rev-parse HEAD 2>/dev/null || true)
    if [ "$have" = "$sha" ]; then
      ok "$name cached ($have)"
      return 0
    fi
  fi
  rm -rf "$dest"
  mkdir -p "$(dirname "$dest")"
  if [ -n "$tag" ] && git clone --depth 1 --branch "$tag" "$url" "$dest"; then
    local have
    have=$(git -C "$dest" rev-parse HEAD)
    if [ "$have" = "$sha" ]; then
      ok "$name at $tag ($sha)"
      return 0
    fi
    ok "$name at tag $tag ($have, expected $sha)"
    return 0
  fi
  rm -rf "$dest"
  if git clone "$url" "$dest" && git -C "$dest" checkout --detach "$sha"; then
    ok "$name at $sha"
    return 0
  fi
  rm -rf "$dest"
  return 1
}

schwung_ok=1
fetch_schwung https://github.com/athousanddetails/schwung-9W9.git \
  a9e030d9bad46e3e53aafce92c7104b8e7bba47f v2.7.2 \
  "$DEPS/schwung/schwung-9W9" schwung-9W9 || schwung_ok=0
fetch_schwung https://github.com/athousanddetails/schwung-8W8.git \
  1dbef27e13a6c63cb1fc078dd9b4d86172237c51 v1.5.1 \
  "$DEPS/schwung/schwung-8W8" schwung-8W8 || schwung_ok=0
fetch_schwung https://github.com/charlesvestal/schwung-303.git \
  1c8e20381c1f23cbe20c618f248ac173b72d913e "" \
  "$DEPS/schwung/schwung-303" schwung-303 || schwung_ok=0
fetch_schwung https://github.com/charlesvestal/schwung-tb3po.git \
  c5d6e1df27e66c35d5653ca9a5a9d99a2a16536b "" \
  "$DEPS/schwung/schwung-tb3po" schwung-tb3po || schwung_ok=0
if [ "$schwung_ok" -ne 1 ]; then
  need "public schwung reference trees (9W9 / 8W8 / 303 / tb3po). Host A/B tests will not-run without them."
fi

if [ -s "$DEPS/MISSING.txt" ]; then
  echo "prepare-deps: missing inputs:"
  cat "$DEPS/MISSING.txt"
else
  ok "all CI dependencies present"
fi
exit 0
